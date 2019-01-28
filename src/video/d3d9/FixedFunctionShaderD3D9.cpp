#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"
#include "video/Pass.h"
#include "video/Renderer.h"

namespace lux
{
namespace video
{

FixedFunctionShaderD3D9::FixedFunctionShaderD3D9(DeviceStateD3D9& deviceState, const FixedFunctionParameters& params) :
	m_DeviceState(deviceState),
	m_TextureStages(params.stages),
	m_UseVertexColors(params.useVertexColors)
{
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

static EFixedLightType FloatToLightType(float type, bool& success)
{
	success = true;
	if(type == 1.0f)
		return EFixedLightType::Directional;
	else if(type == 2.0f)
		return EFixedLightType::Point;
	else if(type == 3.0f)
		return EFixedLightType::Spot;
	else {
		success = false;
		return EFixedLightType::Point;
	}
}

static void ParseLightMatrix(const math::Matrix4& mat,
	video::LightData& data,
	bool& active)
{
	active = (mat(0, 3) != 0.0f);
	bool success;
	data.type = FloatToLightType(mat(0, 3), success);
	if(!success)
		active = false;

	if(!active)
		return;

	data.color.r = mat(0, 0);
	data.color.g = mat(0, 1);
	data.color.b = mat(0, 2);

	data.position.x = mat(1, 0);
	data.position.y = mat(1, 1);
	data.position.z = mat(1, 2);

	data.direction.x = mat(2, 0);
	data.direction.y = mat(2, 1);
	data.direction.z = mat(2, 2);

	data.falloff = mat(3, 0);
	data.innerCone = std::acos(mat(3, 1));
	data.outerCone = std::acos(mat(3, 2));
}

void FixedFunctionShaderD3D9::LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass)
{
	// Connect attributes.
	if(m_CurAttributes != sceneAttributes) {
		m_CurAttributes = sceneAttributes;
		m_AmbientPtr = m_CurAttributes.Pointer("ambient");
		m_FogAPtr = m_CurAttributes.Pointer("fogA");
		m_FogBPtr = m_CurAttributes.Pointer("fogB");

		m_LightPtrs[0] = m_CurAttributes.Pointer("light0");
		m_LightPtrs[1] = m_CurAttributes.Pointer("light1");
		m_LightPtrs[2] = m_CurAttributes.Pointer("light2");
		m_LightPtrs[3] = m_CurAttributes.Pointer("light3");
	}

	m_Lighting = pass.lighting;

	if(m_AmbientPtr)
		m_Ambient = m_AmbientPtr->GetAccess(true).Get<video::ColorF>();
	else
		m_Ambient = video::ColorF(0, 0, 0, 0);

	if(m_FogAPtr && m_FogBPtr) {
		auto fogB = m_FogBPtr->GetAccess(true).Get<video::ColorF>();

		bool enableFog = pass.fogEnabled && fogB.r != 0.0f;
		m_DeviceState.EnableFixedFog(enableFog);
		if(enableFog) {
			auto fogA = m_FogAPtr->GetAccess(true).Get<video::ColorF>();
			auto type =
				fogB.r == 1.0f ? EFixedFogType::Linear :
				fogB.r == 2.0f ? EFixedFogType::Exp :
				fogB.r == 3.0f ? EFixedFogType::ExpSq : EFixedFogType::Linear;
			m_DeviceState.ConfigureFixedFog(
				type, video::ColorF(fogA.r, fogA.g, fogA.b),
				fogB.g, fogB.b, fogB.a);
		}
	} else {
		m_DeviceState.EnableFixedFog(false);
	}

	m_DeviceState.EnableLight(m_Lighting != ELightingFlag::Disabled);
	for(int i = 0; i < 4; ++i) {
		if(m_LightPtrs[i]) {
			video::LightData data;
			bool active;
			ParseLightMatrix(
				m_LightPtrs[i]->GetAccess().Get<math::Matrix4>(),
				data, active);
			if(active)
				m_DeviceState.SetLight(i, data, pass.lighting);
			else
				m_DeviceState.DisableLight(i);
		} else {
			m_DeviceState.DisableLight(i);
		}
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