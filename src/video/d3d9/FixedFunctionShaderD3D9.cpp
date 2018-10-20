#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "video/d3d9/DeviceStateD3D9.h"

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
	for(auto& s : params.textures)
		m_ParamPackage.AddParam(s, TextureLayer());
}

void FixedFunctionShaderD3D9::Enable()
{
	m_IsDirty = true;
}

void FixedFunctionShaderD3D9::SetParam(int paramId, const void* data)
{
	m_Layers[paramId] = *(video::TextureLayer*)data;
	m_IsDirty = true;
}

int FixedFunctionShaderD3D9::GetParamId(core::StringView name) const
{
	return m_ParamPackage.GetParamId(name);
}

void FixedFunctionShaderD3D9::LoadSceneParams(const Pass& pass)
{
	LUX_UNUSED(pass);
}

void FixedFunctionShaderD3D9::Render()
{
	m_DeviceState.EnableFixedFunctionShader(m_Layers, m_TextureStages, m_UseVertexColors);
	m_IsDirty = false;
}

void FixedFunctionShaderD3D9::Disable()
{
}

int FixedFunctionShaderD3D9::GetSceneParamCount() const
{
	return 0;
}

core::AttributePtr FixedFunctionShaderD3D9::GetSceneParam(int id) const
{
	LUX_UNUSED(id);
	return nullptr;
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