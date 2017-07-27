#ifndef INCLUDED_DEVICE_STATE_D3D9_H
#define INCLUDED_DEVICE_STATE_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/DeviceState.h"
#include "video/Color.h"

#include "video/AlphaSettings.h"
#include "video/TextureStageSettings.h"
#include "math/matrix4.h"

#include "video/Shader.h"

#include "StrippedD3D9.h"

namespace lux
{
namespace video
{
class Material;
class FogData;
class LightData;
class BaseTexture;

class DeviceStateD3D9 : public DeviceState
{
public:
	DeviceStateD3D9(IDirect3DDevice9* device);

	void SetD3DColors(const video::Colorf& ambient, const Material& m);
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
	void SetTexture(u32 stage, IDirect3DBaseTexture9* tex);
	void SetTransform(D3DTRANSFORMSTATETYPE type, const math::Matrix4& m);

	void EnableFog(bool enable);
	void SetFog(const FogData& fog);

	void EnableLight(bool enable);
	void ClearLights();
	void AddLight(const LightData& light);

	void SetRenderTargetTexture(video::BaseTexture* t);

	void EnableShader(Shader* s)
	{
		if(s == nullptr && m_Shader)
			m_Shader->Disable();
		else if(s != m_Shader)
			s->Enable();

		m_Shader = s;
	}

private:
	static u32 GetFillMode(const Pass& p);
	static u32 GetCullMode(const Pass& p);
	static u32 Float2U32(float f);

	static u32 GetTextureOperator(ETextureOperator op);
	static u32 GetTextureArgument(ETextureArgument arg);

private:
	D3DMATERIAL9 m_D3DMaterial;
	video::Colorf m_Ambient;

	IDirect3DDevice9* m_Device;
	video::BaseTexture* m_RenderTargetTexture;

	size_t m_LightCount;
	bool m_UseLighting;

	WeakRef<video::Shader> m_Shader;

	size_t m_UsedTextureLayers;
	core::Array<void*> m_Textures;

	EBlendFactor m_SrcBlendFactor;
	EBlendFactor m_DstBlendFactor;
	EBlendOperator m_BlendOperator;

	bool m_UseVertexData;

	bool m_ResetAll;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif // #ifndef INCLUDED_DEVICE_STATE_D3D9_H
