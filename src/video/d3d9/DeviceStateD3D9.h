#ifndef INCLUDED_LUX_DEVICE_STATE_D3D9_H
#define INCLUDED_LUX_DEVICE_STATE_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "math/Matrix4.h"

#include "video/Color.h"
#include "video/TextureStageSettings.h"
#include "video/d3d9/FixedFunctionShaderD3D9.h"

#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{
class Material;
class FogData;
class LightData;
class BaseTexture;
class Pass;

class DeviceStateD3D9
{
public:
	~DeviceStateD3D9();

	void Init(const D3DCAPS9* caps, IDirect3DDevice9* device);

	void EnablePass(const Pass& p);

	void EnableFixedFunctionShader(
		const core::Array<TextureLayer>& layer,
		const core::Array<TextureStageSettings>& settings,
		bool useVertexColor,
		ColorF diffuse, float emissive, float specularHardness, float specularIntensity,
		ColorF ambient, ELightingFlag lighting);

	void EnableTextureLayer(u32 stage, const TextureLayer& layer);
	void EnableTextureStage(u32 stage, const TextureStageSettings& settings, bool useVertexData, ELightingFlag lighting);

	void DisableTexture(u32 stage);

	void EnableAlpha(AlphaBlendMode mode);

	void* GetLowLevelDevice();
	void SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
	void SetRenderStateF(D3DRENDERSTATETYPE state, float value);
	void SetTextureStageState(u32 stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	void SetSamplerState(u32 stage, D3DSAMPLERSTATETYPE state, DWORD value);
	void SetTexture(u32 stage, IDirect3DBaseTexture9* tex);
	void SetTransform(D3DTRANSFORMSTATETYPE type, const math::Matrix4& m);

	void SetFog(const FogData& fog);

	void SetStencilMode(const StencilMode& mode);
	void EnableLight(bool enable);
	void SetLight(u32 id, const LightData& light, ELightingFlag lighting);
	void DisableLight(u32 id);

	void EnableShader(Shader* s)
	{
		if(s != m_Shader && m_Shader)
			m_Shader->Disable();
		if(s) {
			s->Enable();
		}

		m_Shader = s;
	}

	Shader* GetShader() { return m_Shader; }

	void ReleaseUnmanaged();
	void Reset();

private:
	static u32 GetFillMode(const Pass& p);
	static u32 GetCullMode(const Pass& p);
	static u32 Float2U32(float f);

	static u32 GetTextureOperator(ETextureOperator op);
	static u32 GetTextureArgument(ETextureArgument arg);

private:
	const D3DCAPS9* m_Caps;
	D3DMATERIAL9 m_D3DMaterial;

	UnknownRefCounted<IDirect3DDevice9> m_Device;

	WeakRef<video::Shader> m_Shader;

	int m_ActiveTextureLayers;

	AlphaBlendMode m_AlphaMode;

	int m_MaxTextureCount;
	int m_MaxVSTextureCount;

	bool m_ResetAll;

	static const int CACHED_TEXTURES = 8;
	static const int RENDERSTATE_COUNT = 210;
	static const int SAMPLER_STATE_COUNT = 14;
	static const int TEXTURE_STAGE_STATE_COUNT = 33;
	DWORD m_RenderStates[210];
	DWORD m_SamplerStates[CACHED_TEXTURES][14];
	DWORD m_TextureStageStates[CACHED_TEXTURES][33];
	void* m_Textures[CACHED_TEXTURES];
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_LUX_DEVICE_STATE_D3D9_H
