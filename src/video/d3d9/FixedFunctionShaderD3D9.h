#ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#define INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
#include "video/FixedFunctionShader.h"

namespace lux
{
namespace video
{

class DeviceStateD3D9;

class FixedFunctionShaderD3D9 : public Shader
{
public:
	static const int MAX_LIGHT_COUNT = 4;
	/*
	TODO: Record current saved value of FixedFunction pipeline in DeviceState.
	*/
	struct SharedData
	{
		core::AttributePtr ambientPtr;

		core::AttributePtr fogAPtr;
		core::AttributePtr fogBPtr;
		u32 fogChangeId;

		core::AttributePtr matWorldPtr;
		u32 worldChangeId;
		core::AttributePtr matViewPtr;
		core::AttributePtr matProjPtr;
		u32 viewProjChangeId;

		core::AttributePtr lightPtrs[MAX_LIGHT_COUNT];
		u32 lightChangeId;

		core::AttributeList curAttributes;

		void Update(core::AttributeList newAtributes)
		{
			if(newAtributes == curAttributes)
				return;
			curAttributes = newAtributes;
			ambientPtr = curAttributes.Pointer("ambient");
			fogAPtr = curAttributes.Pointer("fogA");
			fogBPtr = curAttributes.Pointer("fogB");

			lightPtrs[0] = curAttributes.Pointer("light0");
			lightPtrs[1] = curAttributes.Pointer("light1");
			lightPtrs[2] = curAttributes.Pointer("light2");
			lightPtrs[3] = curAttributes.Pointer("light3");
			matWorldPtr = curAttributes.Pointer("world");
			matViewPtr = curAttributes.Pointer("view");
			matProjPtr = curAttributes.Pointer("proj");
		}
	};

public:
	FixedFunctionShaderD3D9(DeviceStateD3D9& deviceState, const FixedFunctionParameters& params);
	void Enable() override;
	void SetParam(int paramId, const void* data) override;
	void LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass) override;
	void Render() override;

	const core::ParamPackage& GetParamPackage() const override;

public:
	DeviceStateD3D9& m_DeviceState;

	// Shader settings.
	core::Array<TextureLayer> m_Layers;
	bool m_UseVertexColors;
	core::ParamPackage m_ParamPackage;
	int m_LightCount = 4;
	bool m_UseFog;

	// Params.
	video::ELightingFlag m_Lighting;
	video::ColorF m_Ambient;
	core::Array<TextureStageSettings> m_TextureStages;
	video::ColorF m_Diffuse = video::ColorF(1, 1, 1, 1);
	float m_Emissive = 0.0f;
	float m_SpecularHardness = 0.0f;
	float m_SpecularIntensity = 1.0f;

	SharedData* m_SharedData;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_FIXED_FUNCTION_SHADER_D3D9_H
