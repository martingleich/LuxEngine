#ifndef INCLUDED_LUX_DEVICE_STATE_D3D9_H
#define INCLUDED_LUX_DEVICE_STATE_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "math/Matrix4.h"

#include "video/Color.h"
#include "video/TextureStageSettings.h"
#include "video/d3d9/FixedFunctionShaderD3D9.h"
#include "video/FogData.h"

#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{
class Material;
class BaseTexture;
class Pass;

enum class EFixedFogType
{
	Linear,
	Exp,
	ExpSq,
};
enum class EFixedLightType
{
	Point,
	Directional,
	Spot,
};
//! Contains all data needed to represent a light
class LightData
{
public:
	LightData() :
		type(EFixedLightType::Point),
		color(1.0f, 1.0f, 1.0f),
		position(0.0f, 0.0f, 0.0f),
		direction(0.0f, 0.0f, 1.0f),
		innerCone(math::DegToRad(10.0f)),
		outerCone(math::DegToRad(45.0f)),
		falloff(2.0f)
	{
	}

	//! The light type
	EFixedLightType type;

	//! The color of the light
	/**
	Default: White
	*/
	ColorF color;

	//! The lightposition in world coordinates
	math::Vector3F position;

	//! The lightdirection in world coordinates
	math::Vector3F direction;

	//! The inner lightcone of a spotlight
	/**
	Must be between zero and outerCone
	*/
	float innerCone;

	//! The outer lightcone of a spotlight
	/**
	Must be between innerCone and 2 pi
	*/
	float outerCone;

	//! The falloff between inner and outer light cone
	/**
	Only for spotlights.
	=1: linear falloff
	<1: starts falling slow, becoming faster
	>1: start falling fast, becoming slower
	exact: intensity factor = ((cos a - cos (outerCone/2)) / (cos (innerCone/2) - cos(outerCone/2))) ^ falloff.
	a = Angle between vertex and light direction
	*/
	float falloff;
};

class DeviceStateD3D9
{
public:
	~DeviceStateD3D9();

	void Init(const D3DCAPS9* caps, IDirect3DDevice9* device);

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

	void EnableFixedFog(bool enable);
	void ConfigureFixedFog(EFixedFogType type, const ColorF& color, float start, float end, float density);

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
	static u32 Float2U32(float f);

private:
	const D3DCAPS9* m_Caps;
	D3DMATERIAL9 m_D3DMaterial;

	UnknownRefCounted<IDirect3DDevice9> m_Device;

	WeakRef<video::Shader> m_Shader;

	int m_ActiveTextureLayers;

	AlphaBlendMode m_AlphaMode;

	int m_MaxTextureCount;
	int m_MaxVSTextureCount;

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
