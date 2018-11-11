#ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#define INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#include "video/FixedFunctionShader.h"

namespace lux
{
namespace video
{

class DeviceStateD3D9;
class Renderer;
class FixedFunctionShaderD3D9 : public Shader
{
public:
	FixedFunctionShaderD3D9(Renderer* r, DeviceStateD3D9& deviceState, const FixedFunctionParameters& params);
	void Enable() override;
	void SetParam(int paramId, const void* data) override;
	int GetParamId(core::StringView name) const override;
	void LoadSceneParams(const Pass& pass) override;
	void Render() override;
	void Disable() override;
	int GetSceneParamCount() const override;
	core::AttributePtr GetSceneParam(int id) const override;

	int GetTextureStageCount() const;
	const TextureStageSettings& GetTextureStage(int id) const;
	const core::ParamPackage& GetParamPackage() const;

public:
	DeviceStateD3D9& m_DeviceState;
	core::Array<TextureLayer> m_Layers;
	core::Array<TextureStageSettings> m_TextureStages;
	core::ParamPackage m_ParamPackage;
	video::ColorF m_Diffuse = video::ColorF(1,1,1,1);
	float m_Emissive = 0.0f;
	float m_SpecularHardness = 0.0f;
	float m_SpecularIntensity = 1.0f;
	video::ELightingFlag m_Lighting;
	video::ColorF m_Ambient;
	bool m_UseVertexColors;
	bool m_IsDirty;
	core::AttributePtr m_AmbientPtr;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
