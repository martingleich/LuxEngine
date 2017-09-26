#ifndef INCLUDED_DEVICE_STATE_D3D9_H
#define INCLUDED_DEVICE_STATE_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "math/Matrix4.h"

#include "video/Color.h"
#include "video/TextureStageSettings.h"
#include "video/Shader.h"

#include "StrippedD3D9.h"
#include "UnknownRefCounted.h"

namespace lux
{
namespace video
{
class Material;
class FogData;
class LightData;
class BaseTexture;
class Pass;
class StencilMode;

class DeviceStateD3D9
{
public:
	void Init(const D3DCAPS9* caps, IDirect3DDevice9* device);

	void SetD3DColors(const video::Colorf& ambient, const Pass& pass);
	void EnablePass(const Pass& p);

	void EnableTextureLayer(u32 stage, const TextureLayer& layer);
	void EnableTextureStage(u32 stage, const TextureStageSettings& settings);

	void DisableTexture(u32 stage);
	void EnableVertexData(bool useColor);

	void EnableAlpha(EBlendFactor src, EBlendFactor dst, EBlendOperator op);

	void* GetLowLevelDevice();
	void SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
	void SetRenderStateF(D3DRENDERSTATETYPE state, float value);
	void SetTextureStageState(u32 stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
	void SetSamplerState(u32 stage, D3DSAMPLERSTATETYPE state, DWORD value);
	void SetTexture(u32 stage, IDirect3DBaseTexture9* tex);
	void SetTransform(D3DTRANSFORMSTATETYPE type, const math::Matrix4& m);

	void EnableFog(bool enable);
	void SetFog(const FogData& fog);

	void SetStencilMode(const StencilMode& mode);
	void EnableLight(bool enable);
	void ClearLights();
	void AddLight(const LightData& light, ELighting lighting);

	void EnableShader(Shader* s)
	{
		if(s == nullptr && m_Shader)
			m_Shader->Disable();
		else if(s != m_Shader)
			s->Enable();

		m_Shader = s;
	}

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
	video::Colorf m_Ambient;

	UnknownRefCounted<IDirect3DDevice9> m_Device;

	size_t m_LightCount;
	bool m_UseLighting;

	WeakRef<video::Shader> m_Shader;

	size_t m_UsedTextureLayers;

	EBlendFactor m_SrcBlendFactor;
	EBlendFactor m_DstBlendFactor;
	EBlendOperator m_BlendOperator;

	bool m_UseVertexData;

	bool m_ResetAll;

	static const u32 CACHED_TEXTURES = 8;
	static const u32 RENDERSTATE_COUNT = 210;
	static const u32 SAMPLER_STATE_COUNT = 14;
	static const u32 TEXTURE_STAGE_STATE_COUNT = 33;
	DWORD m_RenderStates[210];
	DWORD m_SamplerStates[CACHED_TEXTURES][14];
	DWORD m_TextureStageStates[CACHED_TEXTURES][33];
	void* m_Textures[CACHED_TEXTURES];
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_DEVICE_STATE_D3D9_H
