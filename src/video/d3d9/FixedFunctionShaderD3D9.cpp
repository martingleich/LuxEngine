#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"
#include "video/Pass.h"
#include "video/Renderer.h"

namespace lux
{
namespace video
{

FixedFunctionShaderD3D9::FixedFunctionShaderD3D9(Renderer* r, DeviceStateD3D9& deviceState, const FixedFunctionParameters& params) :
	m_DeviceState(deviceState),
	m_TextureStages(params.stages),
	m_UseVertexColors(params.useVertexColors)
{
	m_AmbientPtr = r->GetParam("ambient");
	m_Layers.Resize(params.textures.Size());
	core::ParamPackageBuilder ppb;
	ppb.AddParam("diffuse", video::ColorF(1,1,1,1));
	ppb.AddParam("emissive", 0.0f);
	ppb.AddParam("specularHardness", 0.0f);
	ppb.AddParam("specularIntensity", 1.0f);
	for(auto& s : params.textures)
		ppb.AddParam(s, TextureLayer());
	m_ParamPackage = std::move(ppb.Build());
}

void FixedFunctionShaderD3D9::Enable()
{
	m_IsDirty = true;
}

void FixedFunctionShaderD3D9::SetParam(int paramId, const void* data)
{
	switch(paramId) {
	case 0: m_Diffuse = *(video::ColorF*)data; break;
	case 1: m_Emissive = *(float*)data; break;
	case 2: m_SpecularHardness = *(float*)data; break;
	case 3: m_SpecularIntensity = *(float*)data; break;
	default:
		m_Layers[paramId-4] = *(video::TextureLayer*)data;
	}
	m_IsDirty = true;
}

int FixedFunctionShaderD3D9::GetParamId(core::StringView name) const
{
	return m_ParamPackage.GetParamIdByName(name);
}

void FixedFunctionShaderD3D9::LoadSceneParams(const Pass& pass)
{
	m_Lighting = pass.lighting;
	m_Ambient = m_AmbientPtr->GetAccess(true).As<video::ColorF>();
}

void FixedFunctionShaderD3D9::Render()
{
	m_DeviceState.EnableFixedFunctionShader(
		m_Layers,
		m_TextureStages,
		m_UseVertexColors,
		m_Diffuse, m_Emissive, m_SpecularHardness, m_SpecularIntensity,
		m_Ambient, m_Lighting);
	m_IsDirty = false;
}

void FixedFunctionShaderD3D9::Disable()
{
}

int FixedFunctionShaderD3D9::GetSceneParamCount() const
{
	return 1;
}

core::AttributePtr FixedFunctionShaderD3D9::GetSceneParam(int id) const
{
	LX_CHECK_BOUNDS(id, 0, 1);
	return m_AmbientPtr;
}

int FixedFunctionShaderD3D9::GetTextureStageCount() const
{
	return m_TextureStages.Size();
}

const TextureStageSettings& FixedFunctionShaderD3D9::GetTextureStage(int id) const
{
	return m_TextureStages[id];
}

const core::ParamPackage& FixedFunctionShaderD3D9::GetParamPackage() const
{
	return m_ParamPackage;
}

} // namespace video
} // namespace lux