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
	FixedFunctionShaderD3D9(DeviceStateD3D9& deviceState, const FixedFunctionParameters& params);
	void Enable() override;
	void SetParam(int paramId, const void* data) override;
	void LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass) override;
	void Render() override;
	void Disable() override;

	const core::ParamPackage& GetParamPackage() const override;

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
	core::AttributePtr m_FogAPtr;
	core::AttributePtr m_FogBPtr;

	static const int LIGHT_COUNT = 4;
	core::AttributePtr m_LightPtrs[4];

	mutable core::AttributeList m_CurAttributes;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
