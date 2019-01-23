#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "DeviceStateD3D9.h"

#include "video/d3d9/D3DHelper.h"
#include "platform/D3D9Exception.h"

#include "video/Material.h"

#include "video/BaseTexture.h"

namespace lux
{
namespace video
{

static DWORD GetD3DFogType(EFixedFogType type)
{
	switch(type) {
	case EFixedFogType::Exp: return D3DFOG_EXP;
	case EFixedFogType::ExpSq: return D3DFOG_EXP2;
	case EFixedFogType::Linear: return D3DFOG_LINEAR;
	}
	throw core::GenericInvalidArgumentException("type", "Unknown fogtype");
}

void DeviceStateD3D9::Init(const D3DCAPS9* caps, IDirect3DDevice9* device)
{
	m_Caps = caps;
	m_Device = device;
	m_MaxTextureCount = 0;
	m_MaxVSTextureCount = 0;
	Reset();
}

DeviceStateD3D9::~DeviceStateD3D9()
{
	ReleaseUnmanaged();
}

void DeviceStateD3D9::EnableHardwareShader(IDirect3DVertexShader9* vs, IDirect3DPixelShader9* ps)
{
	HRESULT hr;

	if(FAILED(hr = m_Device->SetVertexShader(vs)))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_Device->SetPixelShader(ps)))
		throw core::D3D9Exception(hr);
}

void DeviceStateD3D9::EnableFixedFunctionShader(
	const core::Array<TextureLayer>& layers,
	const core::Array<TextureStageSettings>& stages,
	bool useVertexColors,
	ColorF diffuse, float emissive, float specularHardness, float specularIntensity,
	ColorF ambient, ELightingFlag lighting)
{
	if(lighting != ELightingFlag::Disabled) {
		D3DCOLORVALUE black = {0};
		// Enable d3d material
		D3DMATERIAL9 D3DMaterial = {
			TestFlag(lighting, ELightingFlag::DiffSpec) ? SColorToD3DColor(diffuse) : black,
			TestFlag(lighting, ELightingFlag::AmbientEmit) ? SColorToD3DColor(diffuse) : black,
			TestFlag(lighting, ELightingFlag::DiffSpec) ? SColorToD3DColor(video::ColorF(specularIntensity, specularIntensity, specularIntensity)) : black,
			TestFlag(lighting, ELightingFlag::AmbientEmit) ? SColorToD3DColor(emissive * diffuse) : black,
			TestFlag(lighting, ELightingFlag::DiffSpec) ? specularHardness : 0.0f
		};

		m_D3DMaterial = D3DMaterial;
		m_Device->SetMaterial(&m_D3DMaterial);
	}
	if(TestFlag(lighting, ELightingFlag::AmbientEmit))
		SetRenderState(D3DRS_AMBIENT, ambient.ToHex());
	else
		SetRenderState(D3DRS_AMBIENT, 0);

	if(TestFlag(lighting, ELightingFlag::DiffSpec) && !math::IsZero(m_D3DMaterial.Power))
		SetRenderState(D3DRS_SPECULARENABLE, 1);
	else
		SetRenderState(D3DRS_SPECULARENABLE, 0);
	if(lighting == ELightingFlag::Disabled)
		SetRenderState(D3DRS_TEXTUREFACTOR, diffuse.ToHex());

	SetRenderState(D3DRS_COLORVERTEX, useVertexColors ? TRUE : FALSE);

	int layerCount = layers.Size();
	if(layerCount == 0)
		layerCount = 1;
	// Enable texture layers for fixed function pipeline
	// Shaders handle this while loading their parameters.
	static const TextureStageSettings DEFAULT_STAGE;
	static const TextureStageSettings DIFFUSE_ONLY_STAGE(
		ETextureArgument::Diffuse,
		ETextureArgument::Diffuse,
		ETextureOperator::SelectArg1,
		ETextureArgument::Diffuse,
		ETextureArgument::Diffuse,
		ETextureOperator::SelectArg1);
	static const TextureLayer EMPTY_LAYER;
	for(int i = 0; i < layerCount; ++i) {
		auto& tex = i < layers.Size() ? layers[i] : EMPTY_LAYER;
		EnableTextureLayer(i, tex);

		const TextureStageSettings* settings;
		if(!tex.texture)
			settings = &DIFFUSE_ONLY_STAGE;
		else
			settings = i < stages.Size() ? &stages[i] : &DEFAULT_STAGE;

		EnableTextureStage(i, *settings, useVertexColors, lighting);
	}

	for(int i = layerCount; i < stages.Size(); ++i)
		EnableTextureStage(i, stages[i], useVertexColors, lighting);

	auto newUsed = math::Max(layerCount, stages.Size());
	for(int i = newUsed; i < m_ActiveTextureLayers; ++i)
		DisableTexture(i);

	m_ActiveTextureLayers = newUsed;
}

void DeviceStateD3D9::EnableTextureLayer(u32 stage, const TextureLayer& layer)
{
	bool textureSet = false;
	if(layer.texture) {
		SetTexture(stage, (IDirect3DBaseTexture9*)(layer.texture->GetRealTexture()));
		textureSet = true;
	} else {
		SetTexture(stage, nullptr);
	}
	if(stage >= D3DVERTEXTEXTURESAMPLER0)
		m_MaxVSTextureCount = math::Max<int>(stage - D3DVERTEXTEXTURESAMPLER0 + 1, m_MaxVSTextureCount);
	else
		m_MaxTextureCount = math::Max<int>(stage + 1, m_MaxTextureCount);

	if(textureSet) {
		SetSamplerState(stage, D3DSAMP_ADDRESSU, GetD3DRepeatMode(layer.repeat.u));
		SetSamplerState(stage, D3DSAMP_ADDRESSV, GetD3DRepeatMode(layer.repeat.v));
		SetSamplerState(stage, D3DSAMP_BORDERCOLOR, layer.repeat.border.ToDWORD());

		BaseTexture::Filter filter = layer.texture->GetFiltering();

		// TODO: Make default filter configurable
		if(filter.magFilter == BaseTexture::Filter::Any)
			filter.magFilter = BaseTexture::Filter::Linear;
		if(filter.minFilter == BaseTexture::Filter::Any)
			filter.minFilter = BaseTexture::Filter::Linear;
		if(filter.mipFilter == BaseTexture::Filter::Any)
			filter.mipFilter = BaseTexture::Filter::Linear;

		u32 filterMag = GetD3DTextureFilter(filter.magFilter);
		u32 filterMin = GetD3DTextureFilter(filter.minFilter);
		u32 filterMip = filter.mipFilter == BaseTexture::Filter::Linear ? D3DTEXF_LINEAR : D3DTEXF_POINT;

		SetSamplerState(stage, D3DSAMP_MIPFILTER, filterMip);
#if 0
		if(filterMag == D3DTEXF_ANISOTROPIC || filterMin == D3DTEXF_ANISOTROPIC) {
			u16 ani = settings.pipeline.Anisotropic;
			u16 maxAni = (u16)m_Driver->GetDeviceCapability(video::EDriverCaps::MaxAnisotropy);
			if(ani == 0)
				ani = maxAni;
			if(ani > maxAni)
				ani = maxAni;
			SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, ani);
	}
#endif
		SetSamplerState(stage, D3DSAMP_MINFILTER, filterMin);
		SetSamplerState(stage, D3DSAMP_MAGFILTER, filterMag);
}
}

void DeviceStateD3D9::EnableTextureStage(
	u32 stage, 
	const TextureStageSettings& settings,
	bool useVertexData, ELightingFlag lighting)
{
	if(lighting == ELightingFlag::Disabled && settings.colorArg1 == ETextureArgument::Diffuse && !useVertexData) {
		SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	} else {
		SetTextureStageState(stage, D3DTSS_COLORARG1, GetD3DTextureArgument(settings.colorArg1));
		SetTextureStageState(stage, D3DTSS_ALPHAARG1, GetD3DTextureArgument(settings.alphaArg1));
	}
	if(lighting == ELightingFlag::Disabled && settings.colorArg2 == ETextureArgument::Diffuse && !useVertexData) {
		SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	} else {
		SetTextureStageState(stage, D3DTSS_COLORARG2, GetD3DTextureArgument(settings.colorArg2));
		SetTextureStageState(stage, D3DTSS_ALPHAARG2, GetD3DTextureArgument(settings.alphaArg2));
	}

	SetTextureStageState(stage, D3DTSS_COLOROP, GetD3DTextureOperator(settings.colorOperator));
	SetTextureStageState(stage, D3DTSS_ALPHAOP, GetD3DTextureOperator(settings.alphaOperator));

	if(settings.HasAlternateCoordSource())
		SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, settings.coordSource);
}

void DeviceStateD3D9::DisableTexture(u32 stage)
{
	SetTexture(stage, nullptr);
	SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, stage);
}

void DeviceStateD3D9::EnableAlpha(AlphaBlendMode mode)
{
	if(m_AlphaMode == mode)
		return;

	if(mode.blendOperator == EBlendOperator::None) {
		SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	} else {
		SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		SetRenderState(D3DRS_SRCBLEND, GetD3DBlend(mode.srcFactor));
		SetRenderState(D3DRS_DESTBLEND, GetD3DBlend(mode.dstFactor));
		SetRenderState(D3DRS_BLENDOP, GetD3DBlendFunc(mode.blendOperator));
	}

	m_AlphaMode = mode;
}

IDirect3DDevice9* DeviceStateD3D9::GetLowLevelDevice()
{
	return m_Device;
}

void DeviceStateD3D9::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	if(m_RenderStates[(int)state] != value) {
		m_RenderStates[(int)state] = value;
		m_Device->SetRenderState(state, value);
	}
}

void DeviceStateD3D9::SetRenderStateF(D3DRENDERSTATETYPE state, float value)
{
	DWORD v;
	memcpy(&v, &value, 4);
	SetRenderState(state, v);
}

void DeviceStateD3D9::SetTextureStageState(u32 stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	if(stage >= CACHED_TEXTURES || m_TextureStageStates[stage][(int)state] != value) {
		HRESULT hr = m_Device->SetTextureStageState(stage, state, value);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
		if(stage < CACHED_TEXTURES)
			m_TextureStageStates[stage][(int)state] = value;
	}
}

void DeviceStateD3D9::SetSamplerState(u32 stage, D3DSAMPLERSTATETYPE state, DWORD value)
{
	if(stage >= CACHED_TEXTURES || m_SamplerStates[stage][(int)state] != value) {
		HRESULT hr = m_Device->SetSamplerState(stage, state, value);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
		if(stage < CACHED_TEXTURES)
			m_SamplerStates[stage][(int)state] = value;
	}
}

void DeviceStateD3D9::SetTexture(u32 stage, IDirect3DBaseTexture9* tex)
{
	if(stage >= CACHED_TEXTURES || tex != m_Textures[stage]) {
		HRESULT hr = m_Device->SetTexture(stage, tex);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);

		if(stage < CACHED_TEXTURES)
			m_Textures[stage] = tex;
	}
}

void DeviceStateD3D9::SetTransform(D3DTRANSFORMSTATETYPE type, const math::Matrix4& m)
{
	m_Device->SetTransform(type, (D3DMATRIX*)m.DataRowMajor());
}

u32 DeviceStateD3D9::Float2U32(float f)
{
	u32 out;
	static_assert(sizeof(out) == sizeof(f), "Float isn't 4 byte big");
	memcpy(&out, &f, 4);

	return out;
}

void DeviceStateD3D9::EnableFixedFog(bool enabled)
{
	SetRenderState(D3DRS_FOGENABLE, enabled ? TRUE : FALSE);
}

void DeviceStateD3D9::ConfigureFixedFog(EFixedFogType type, const ColorF& color, float start, float end, float density)
{
	DWORD d3dType = GetD3DFogType(type);

	// TODO: Handle per pixel fog
	SetRenderState(D3DRS_FOGVERTEXMODE, d3dType);
	SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);

	SetRenderState(D3DRS_FOGCOLOR, ColorFToColor(color).ToDWORD());
	SetRenderStateF(D3DRS_FOGSTART, start);
	SetRenderStateF(D3DRS_FOGEND, end);
	SetRenderStateF(D3DRS_FOGDENSITY, density);
}

void DeviceStateD3D9::EnableLight(bool enable)
{
	SetRenderState(D3DRS_LIGHTING, enable ? TRUE : FALSE);
}

void DeviceStateD3D9::DisableLight(u32 id)
{
	m_Device->LightEnable((DWORD)id, FALSE);
}

void DeviceStateD3D9::SetStencilMode(const StencilMode& mode)
{
	bool isEnabled = mode.IsEnabled();
	SetRenderState(D3DRS_STENCILENABLE, isEnabled ? TRUE : FALSE);
	if(isEnabled) {
		SetRenderState(D3DRS_STENCILFUNC, GetD3DComparisonFunc(mode.test));

		SetRenderState(D3DRS_STENCILREF, mode.ref);
		SetRenderState(D3DRS_STENCILMASK, mode.readMask);
		SetRenderState(D3DRS_STENCILWRITEMASK, mode.writeMask);

		SetRenderState(D3DRS_STENCILPASS, GetD3DStencilOperator(mode.pass));
		SetRenderState(D3DRS_STENCILFAIL, GetD3DStencilOperator(mode.fail));
		SetRenderState(D3DRS_STENCILZFAIL, GetD3DStencilOperator(mode.zFail));

		bool isTwosided = mode.IsTwoSided();
		SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, isTwosided ? TRUE : FALSE);
		if(isTwosided) {
			SetRenderState(D3DRS_CCW_STENCILPASS, GetD3DStencilOperator(mode.passCCW));
			SetRenderState(D3DRS_CCW_STENCILFAIL, GetD3DStencilOperator(mode.failCCW));
			SetRenderState(D3DRS_CCW_STENCILZFAIL, GetD3DStencilOperator(mode.zFailCCW));
		}
	}
}

void DeviceStateD3D9::SetLight(u32 id, const LightData& light, ELightingFlag lighting)
{
	DWORD lightId = (DWORD)id;

	D3DLIGHT9 D3DLight;

	switch(light.type) {
	case EFixedLightType::Point:
		D3DLight.Type = D3DLIGHT_POINT;
		break;
	case EFixedLightType::Spot:
		D3DLight.Type = D3DLIGHT_SPOT;
		break;
	case EFixedLightType::Directional:
		D3DLight.Type = D3DLIGHT_DIRECTIONAL;
		break;
	}

	D3DLight.Position = *((D3DVECTOR*)(&light.position));
	D3DLight.Direction = *((D3DVECTOR*)(&light.direction));

	D3DLight.Range = std::sqrt(FLT_MAX);
	D3DLight.Falloff = light.falloff;

	D3DCOLORVALUE specular = {1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE ambient = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DCOLORVALUE black = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DLight.Diffuse = TestFlag(lighting, ELightingFlag::DiffSpec) ? SColorToD3DColor(light.color) : black;
	D3DLight.Specular = TestFlag(lighting, ELightingFlag::DiffSpec) ? SColorToD3DColor(light.color) : black;
	D3DLight.Ambient = TestFlag(lighting, ELightingFlag::AmbientEmit) ? ambient : black;

	D3DLight.Attenuation0 = 0.0f;
	D3DLight.Attenuation1 = 1.0f;
	D3DLight.Attenuation2 = 0.0f;

	D3DLight.Theta = light.innerCone * 2.0f;
	D3DLight.Phi = light.outerCone * 2.0f;

	HRESULT hr;
	if(FAILED(hr = m_Device->SetLight(lightId, &D3DLight)))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_Device->LightEnable(lightId, TRUE)))
		throw core::D3D9Exception(hr);
}

void DeviceStateD3D9::ReleaseUnmanaged()
{
	for(int i = 0; i < m_MaxTextureCount; ++i)
		m_Device->SetTexture(i, nullptr);
	for(int i = 0; i < m_MaxVSTextureCount; ++i)
		m_Device->SetTexture(D3DVERTEXTEXTURESAMPLER0 + i, nullptr);
	m_Shader = nullptr;
	m_Device->SetPixelShader(nullptr);
	m_Device->SetVertexShader(nullptr);
}

void DeviceStateD3D9::Reset()
{
	m_Shader = nullptr;
	m_ActiveTextureLayers = 0;

	for(auto i = 0; i < RENDERSTATE_COUNT; ++i)
		m_Device->GetRenderState((D3DRENDERSTATETYPE)i, m_RenderStates + i);

	for(auto layer = 0; layer < CACHED_TEXTURES; ++layer) {
		m_Textures[layer] = nullptr;
		for(auto i = 0; i < TEXTURE_STAGE_STATE_COUNT; ++i)
			m_Device->GetTextureStageState(layer, (D3DTEXTURESTAGESTATETYPE)i, m_TextureStageStates[layer] + i);

		for(auto i = 0; i < SAMPLER_STATE_COUNT; ++i)
			m_Device->GetSamplerState(layer, (D3DSAMPLERSTATETYPE)i, m_SamplerStates[layer] + i);
	}
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
