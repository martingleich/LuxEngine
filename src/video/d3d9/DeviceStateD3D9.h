#ifndef INCLUDED_DEVICE_STATE_D3D9_H
#define INCLUDED_DEVICE_STATE_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/DeviceState.h"
#include "video/Color.h"

#include "video/PipelineSettings.h"
#include "video/AlphaSettings.h"
#include "video/TextureStageSettings.h"
#include "math/matrix4.h"

#include "video/Shader.h"

#include "StrippedD3D9.h"

namespace lux
{
namespace video
{
class PipelineSettings;
class Material;
class FogData;
class LightData;
class BaseTexture;

class DeviceStateD3D9 : public DeviceState
{
public:
	DeviceStateD3D9(IDirect3DDevice9* device);
	void EnablePipeline(const PipelineSettings& pipeline);

	void EnableTextureLayer(u32 stage, const TextureLayer& layer);
	void EnableTextureStage(u32 stage, const TextureStageSettings& settings);
	void DisableTextureStage(u32 stage);

	void EnableVertexData();
	void DisableVertexData();

	void EnableAlpha(const AlphaBlendSettings& settings);
	void DisableAlpha();
	void* GetLowLevelDevice();
	void SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
	void SetRenderStateF(D3DRENDERSTATETYPE state, float value);
	void SetTextureStageState(u32 stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	void SetD3DMaterial(const video::Colorf& ambient, const PipelineSettings& pipeline, const video::Material* mat);
	void SetTransform(D3DTRANSFORMSTATETYPE type, const math::matrix4& m);
	void SetFog(bool active, const FogData& fog);
	void ClearLights(bool useLights);
	void EnableLight(const LightData& light);
	void SetRenderTargetTexture(video::BaseTexture* t);

	void EnableShader(Shader* s)
	{
		if(s != m_CurShader)
			s->Enable();
		m_CurShader = s;
	}

	void DisableCurShader()
	{
		m_Device->SetVertexShader(NULL);
		m_Device->SetPixelShader(NULL);
		m_CurShader = nullptr;
	}

private:
	static u32 GetFillMode(const PipelineSettings& pipeline);
	static u32 GetCullMode(const PipelineSettings& pipeline);
	static u32 Float2U32(float f);

	static u32 GetTextureOperator(ETextureOperator op);
	static u32 GetTextureArgument(ETextureArgument arg);

private:
	IDirect3DDevice9* m_Device;
	video::BaseTexture* m_RenderTargetTexture;
	WeakRef<video::Shader> m_CurShader;

	size_t m_LightCount;

	PipelineSettings m_CurPipeline;
	AlphaBlendSettings m_CurAlpha;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_DEVICE_STATE_D3D9_H
