#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"
#include "video/Pass.h"
#include "video/Renderer.h"

#include "video/d3d9/D3DHelper.h"

namespace lux
{
namespace video
{

static FixedFunctionShaderD3D9::SharedData g_SharedData;
static DWORD GetD3DFogType(EFixedFogType type)
{
	switch(type) {
	case EFixedFogType::Exp: return D3DFOG_EXP;
	case EFixedFogType::ExpSq: return D3DFOG_EXP2;
	case EFixedFogType::Linear: return D3DFOG_LINEAR;
	}
	throw core::GenericInvalidArgumentException("type", "Unknown fogtype");
}

FixedFunctionShaderD3D9::FixedFunctionShaderD3D9(DeviceStateD3D9& deviceState, const FixedFunctionParameters& params) :
	m_DeviceState(deviceState),
	m_TextureStages(params.stages),
	m_UseVertexColors(params.useVertexColors)
{
	m_SharedData = &g_SharedData;

	m_Layers.Resize(params.textures.Size());
	core::ParamPackageBuilder ppb;
	ppb.AddParam("diffuse", video::ColorF(1, 1, 1, 1));
	ppb.AddParam("emissive", 0.0f);
	ppb.AddParam("specularHardness", 0.0f);
	ppb.AddParam("specularIntensity", 1.0f);
	for(auto& s : params.textures)
		ppb.AddParam(s, TextureLayer());
	m_ParamPackage = std::move(ppb.Build());
	m_LightCount = params.maxLightCount;
	lxAssert(m_LightCount <= 4);
	m_UseFog = params.enableFogging;
}

void FixedFunctionShaderD3D9::Enable()
{
	m_DeviceState.EnableHardwareShader(nullptr, nullptr);
}

void FixedFunctionShaderD3D9::SetParam(int paramId, const void* data)
{
	switch(paramId) {
	case 0: m_Diffuse = *(video::ColorF*)data; break;
	case 1: m_Emissive = *(float*)data; break;
	case 2: m_SpecularHardness = *(float*)data; break;
	case 3: m_SpecularIntensity = *(float*)data; break;
	default:
		m_Layers.At(paramId - 4) = *(video::TextureLayer*)data;
	}
}

static bool FloatToLightType(float type, D3DLIGHTTYPE& out)
{
	if(type == 1.0f)
		out = D3DLIGHT_DIRECTIONAL;
	else if(type == 2.0f)
		out = D3DLIGHT_POINT;
	else if(type == 3.0f)
		out = D3DLIGHT_SPOT;
	else
		return false;
	return true;
}

void FixedFunctionShaderD3D9::LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass)
{
	// Reconnect attributes, if neccessary
	m_SharedData->Update(sceneAttributes);

	m_Lighting = pass.lighting;

	// Ambient
	if(m_SharedData->ambientPtr)
		m_Ambient = m_SharedData->ambientPtr->GetValue<video::ColorF>();
	else
		m_Ambient = video::ColorF(0, 0, 0, 0);

	// Update fog
	{
		auto aptr = m_SharedData->fogAPtr;
		auto bptr = m_SharedData->fogBPtr;
		if(m_UseFog && aptr && bptr) {
			auto fogId = aptr->GetChangeId() + bptr->GetChangeId();
			auto fogB = bptr->GetValue<video::ColorF>();
			bool enableFog = pass.fogEnabled && fogB.r != 0.0f;
			m_DeviceState.SetRenderStateB(D3DRS_FOGENABLE, enableFog);
			if(enableFog && fogId != m_SharedData->fogChangeId) {
				m_SharedData->fogChangeId = fogId;
				auto fogA = aptr->GetValue<video::ColorF>();
				auto type =
					fogB.r == 1.0f ? EFixedFogType::Linear :
					fogB.r == 2.0f ? EFixedFogType::Exp :
					fogB.r == 3.0f ? EFixedFogType::ExpSq : EFixedFogType::Linear;
				auto fogColor = video::ColorF(fogA.r, fogA.g, fogA.b);
				DWORD d3dType = GetD3DFogType(type);

				// TODO: Handle per pixel fog
				m_DeviceState.SetRenderState(D3DRS_FOGVERTEXMODE, d3dType);
				m_DeviceState.SetRenderStateB(D3DRS_RANGEFOGENABLE, true);

				m_DeviceState.SetRenderState(D3DRS_FOGCOLOR, ColorFToColor(fogColor).ToDWORD());
				m_DeviceState.SetRenderStateF(D3DRS_FOGSTART, fogB.g);
				m_DeviceState.SetRenderStateF(D3DRS_FOGEND, fogB.b);
				m_DeviceState.SetRenderStateF(D3DRS_FOGDENSITY, fogB.a);
			}
		} else {
			m_DeviceState.SetRenderStateB(D3DRS_FOGENABLE, false);
		}
	}

	// Update lights
	{
		m_DeviceState.SetRenderStateB(D3DRS_LIGHTING, pass.lighting != ELightingFlag::Disabled);
		u32 lightChangeId = 0;
		for(int i = 0; i < MAX_LIGHT_COUNT; ++i) {
			auto lptr = m_SharedData->lightPtrs[i];
			lightChangeId += lptr->GetChangeId();
		}

		if(lightChangeId != m_SharedData->lightChangeId) {
			m_SharedData->lightChangeId = lightChangeId;

			for(int i = 0; i < MAX_LIGHT_COUNT; ++i) {
				auto lptr = m_SharedData->lightPtrs[i];
				if(!lptr) {
					m_DeviceState.DisableLight(i);
					continue;
				}

				auto& mat = lptr->GetValue<math::Matrix4>();
				bool active = (mat(0, 3) != 0.0f);
				D3DLIGHT9 D3DLight;
				bool success = FloatToLightType(mat(0, 3), D3DLight.Type);
				if(!success)
					active = false;

				if(!active) {
					m_DeviceState.DisableLight(i);
					continue;
				}

				D3DLight.Attenuation0 = 0.0f;
				D3DLight.Attenuation1 = 1.0f;
				D3DLight.Attenuation2 = 0.0f;

				D3DCOLORVALUE black = {0.0f, 0.0f, 0.0f, 0.0f};
				if(TestFlag(pass.lighting, ELightingFlag::DiffSpec)) {
					D3DLight.Diffuse.a = D3DLight.Specular.a = 1.0f;
					D3DLight.Diffuse.r = D3DLight.Specular.r = mat(0, 0);
					D3DLight.Diffuse.g = D3DLight.Specular.g = mat(0, 1);
					D3DLight.Diffuse.b = D3DLight.Specular.b = mat(0, 2);
				} else {
					D3DLight.Diffuse = D3DLight.Specular = black;
				}
				D3DLight.Ambient = black;

				D3DLight.Position.x = mat(1, 0);
				D3DLight.Position.y = mat(1, 1);
				D3DLight.Position.z = mat(1, 2);

				D3DLight.Direction.x = mat(2, 0);
				D3DLight.Direction.y = mat(2, 1);
				D3DLight.Direction.z = mat(2, 2);

				D3DLight.Range = std::sqrt(FLT_MAX);
				if(D3DLight.Type == D3DLIGHT_SPOT) {
					D3DLight.Falloff = mat(3, 0);
					D3DLight.Theta = std::acos(mat(3, 1)) * 2;
					D3DLight.Phi = std::acos(mat(3, 2)) * 2;
				}

				m_DeviceState.SetAndEnableLight(i, D3DLight);
			}
		}
	}

	// Update transforms
	auto worldId = m_SharedData->matWorldPtr->GetChangeId();
	if(worldId != m_SharedData->worldChangeId) {
		m_SharedData->worldChangeId = worldId;
		m_DeviceState.SetTransform(D3DTS_WORLD, m_SharedData->matWorldPtr->GetValue<math::Matrix4>());
	}
	auto viewProjId = m_SharedData->matProjPtr->GetChangeId() + m_SharedData->matViewPtr->GetChangeId();
	if(viewProjId != m_SharedData->viewProjChangeId) {
		m_SharedData->viewProjChangeId = viewProjId;
		m_DeviceState.SetTransform(D3DTS_VIEW, m_SharedData->matViewPtr->GetValue<math::Matrix4>());
		m_DeviceState.SetTransform(D3DTS_PROJECTION, m_SharedData->matProjPtr->GetValue<math::Matrix4>());
	}
}

void FixedFunctionShaderD3D9::Render()
{
	m_DeviceState.EnableFixedFunctionShader(
		m_Layers,
		m_TextureStages,
		m_UseVertexColors,
		m_Diffuse, m_Emissive, m_SpecularHardness, m_SpecularIntensity,
		m_Ambient, m_Lighting);
}

const core::ParamPackage& FixedFunctionShaderD3D9::GetParamPackage() const
{
	return m_ParamPackage;
}

} // namespace video
} // namespace lux