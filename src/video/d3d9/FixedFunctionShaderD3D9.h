#ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#define INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#include "video/FixedFunctionShader.h"
#include "video/d3d9/DeviceStateD3D9.h"

namespace lux
{
namespace video
{

class FixedFunctionShaderD3D9 : public FixedFunctionShader
{
public:
	FixedFunctionShaderD3D9(DeviceStateD3D9& deviceState,
		const FixedFunctionParameters& params) :
		FixedFunctionShader(params),
		m_DeviceState(deviceState)
	{
		m_Layers.Resize(params.textures.Size());
	}

	void Enable() override
	{
		m_IsDirty = true;
	}

	void SetParam(int paramId, const void* data) override
	{
		m_Layers[paramId] = *(video::TextureLayer*)data;
		m_IsDirty = true;
	}

	int GetParamId(core::StringView name) const override
	{
		return m_ParamPackage.GetParamId(name);
	}

	void LoadSceneParams(const Pass& pass) override
	{
		LUX_UNUSED(pass);
	}

	void Render() override
	{
		m_DeviceState.EnableFixedFunctionShader(m_Layers, m_TextureStages, m_UseVertexColors);
		m_IsDirty = false;
	}

	void Disable() override
	{
	}

	int GetSceneParamCount() const override
	{
		return 0;
	}

	core::AttributePtr GetSceneParam(int id) const override
	{
		LUX_UNUSED(id);
		return nullptr;
	}

public:
	DeviceStateD3D9& m_DeviceState;
	core::Array<TextureLayer> m_Layers;
	bool m_IsDirty;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
