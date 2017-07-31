#ifdef LUX_COMPILE_WITH_D3D9
#include "DeviceStateD3D9.h"

#include "video/d3d9/D3DHelper.h"
#include "video/d3d9/D3D9Exception.h"

#include "video/FogData.h"
#include "video/LightData.h"
#include "video/Material.h"

#include "video/BaseTexture.h"
#include "video/MaterialRenderer.h"

namespace lux
{
namespace video
{

DeviceStateD3D9::DeviceStateD3D9(IDirect3DDevice9* device) :
	m_Device(device),
	m_RenderTargetTexture(nullptr),
	m_Shader(nullptr),
	m_LightCount(0),
	m_UsedTextureLayers(0),
	m_UseVertexData(true),
	m_ResetAll(true)
{
}

void DeviceStateD3D9::SetD3DColors(const video::Colorf& ambient, const Material& m, ELighting lighting)
{
	D3DCOLORVALUE black = {0};
	// Enable d3d material
	D3DMATERIAL9 D3DMaterial = {
		TestFlag(lighting, ELighting::Diffuse) ? SColorToD3DColor(m.GetDiffuse()) : black,
		TestFlag(lighting, ELighting::Ambient) ? SColorToD3DColor(m.GetDiffuse()*m.GetAmbient()) : black,
		TestFlag(lighting, ELighting::Specular) ? SColorToD3DColor(m.GetSpecular()*m.GetPower()) : black,
		TestFlag(lighting, ELighting::Emissive) ? SColorToD3DColor(m.GetEmissive()) : black,
		TestFlag(lighting, ELighting::Specular) ? m.GetShininess() : 0.0f
	};

	m_D3DMaterial = D3DMaterial;
	m_Device->SetMaterial(&m_D3DMaterial);
	m_Ambient = ambient;

	SetRenderState(D3DRS_TEXTUREFACTOR, m.GetDiffuse().ToHex());
}

void DeviceStateD3D9::EnablePass(const Pass& p)
{
	m_UseLighting = (p.lighting != ELighting::Disabled);

	EnableShader(p.shader);

	// Apply overwrite and enable pipeline settings.
	EnableAlpha(p.alphaSrcBlend, p.alphaDstBlend, p.alphaOperator);
	EnableVertexData(p.useVertexColor);

	// Enable layers
	static const TextureStageSettings DEFAULT_STAGE;
	static const TextureStageSettings DIFFUSE_ONLY_STAGE(
		ETextureArgument::Diffuse,
		ETextureArgument::Diffuse,
		ETextureOperator::SelectArg1,
		ETextureArgument::Diffuse,
		ETextureArgument::Diffuse,
		ETextureOperator::SelectArg1);
	for(size_t i = 0; i < p.layers.Size(); ++i) {
		EnableTextureLayer(i, p.layers[i]);

		const TextureStageSettings* settings;
		if(!p.layers[i].texture)
			settings = &DIFFUSE_ONLY_STAGE;
		else
			settings = i < p.layerSettings.Size() ? &p.layerSettings[i] : &DEFAULT_STAGE;

		EnableTextureStage(i, *settings);
	}

	for(size_t i = p.layers.Size(); i < p.layerSettings.Size(); ++i)
		EnableTextureStage(i, p.layerSettings[i]);

	// Disable old layers
	for(size_t i = p.layers.Size(); i < m_UsedTextureLayers; ++i)
		DisableTexture(i);

	m_UsedTextureLayers = math::Max(p.layers.Size(), p.layerSettings.Size());

	// Set Material parameters
	if(p.lighting == ELighting::Enabled || TestFlag(p.lighting, ELighting::Ambient))
		SetRenderState(D3DRS_AMBIENT, m_Ambient.ToHex());
	else
		SetRenderState(D3DRS_AMBIENT, 0);

	if((p.lighting == ELighting::Enabled || TestFlag(p.lighting, ELighting::Specular)) && !math::IsZero(m_D3DMaterial.Power))
		SetRenderState(D3DRS_SPECULARENABLE, 1);
	else
		SetRenderState(D3DRS_SPECULARENABLE, 0);

	m_Device->SetRenderState(D3DRS_ZFUNC, GetD3DComparisonFunc(p.zBufferFunc));
	m_Device->SetRenderState(D3DRS_ZWRITEENABLE, p.zWriteEnabled ? TRUE : FALSE);
	m_Device->SetRenderState(D3DRS_NORMALIZENORMALS, p.normalizeNormals ? TRUE : FALSE);
	m_Device->SetRenderState(D3DRS_FILLMODE, GetFillMode(p));
	m_Device->SetRenderState(D3DRS_SHADEMODE, p.gouraudShading ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
	m_Device->SetRenderState(D3DRS_CULLMODE, GetCullMode(p));

	m_ResetAll = false;
}

void DeviceStateD3D9::EnableTextureLayer(u32 stage, const TextureLayer& layer)
{
	if(layer.texture) {
		// The current rendertarget can't be used as texture -> set texture channel to black
		// But tell all people asking that the correct texture was set.
		if(layer.texture == m_RenderTargetTexture) {
			SetTexture(stage, nullptr);
		} else {
			SetTexture(stage, (IDirect3DBaseTexture9*)(layer.texture->GetRealTexture()));
		}
	} else {
		SetTexture(stage, nullptr);
	}

	if(m_Textures[stage]) {
		m_Device->SetSamplerState(stage, D3DSAMP_ADDRESSU, GetD3DRepeatMode(layer.repeat.u));
		m_Device->SetSamplerState(stage, D3DSAMP_ADDRESSV, GetD3DRepeatMode(layer.repeat.v));

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

		m_Device->SetSamplerState(stage, D3DSAMP_MIPFILTER, filterMip);
#if 0
		if(filterMag == D3DTEXF_ANISOTROPIC || filterMin == D3DTEXF_ANISOTROPIC) {
			u16 ani = settings.pipeline.Anisotropic;
			u16 maxAni = (u16)m_Driver->GetDeviceCapability(video::EDriverCaps::MaxAnisotropy);
			if(ani == 0)
				ani = maxAni;
			if(ani > maxAni)
				ani = maxAni;
			m_Device->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, ani);
	}
#endif
		m_Device->SetSamplerState(stage, D3DSAMP_MINFILTER, filterMin);
		m_Device->SetSamplerState(stage, D3DSAMP_MAGFILTER, filterMag);
}
}

void DeviceStateD3D9::EnableTextureStage(u32 stage, const TextureStageSettings& settings)
{
	if(!m_UseLighting && settings.colorArg1 == ETextureArgument::Diffuse && !m_UseVertexData)
		SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	else
		SetTextureStageState(stage, D3DTSS_COLORARG1, GetTextureArgument(settings.colorArg1));
	if(!m_UseLighting && settings.colorArg2 == ETextureArgument::Diffuse && !m_UseVertexData)
		SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	else
		SetTextureStageState(stage, D3DTSS_COLORARG2, GetTextureArgument(settings.colorArg2));

	SetTextureStageState(stage, D3DTSS_COLOROP, GetTextureOperator(settings.colorOperator));

	SetTextureStageState(stage, D3DTSS_ALPHAARG1, GetTextureArgument(settings.alphaArg1));
	SetTextureStageState(stage, D3DTSS_ALPHAARG2, GetTextureArgument(settings.alphaArg2));
	SetTextureStageState(stage, D3DTSS_ALPHAOP, GetTextureOperator(settings.alphaOperator));

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

void DeviceStateD3D9::EnableAlpha(EBlendFactor src, EBlendFactor dst, EBlendOperator op)
{
	if(m_SrcBlendFactor == src && m_DstBlendFactor == dst && m_BlendOperator == op && !m_ResetAll)
		return;

	if(op == EBlendOperator::None) {
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	} else {
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_Device->SetRenderState(D3DRS_DESTBLEND, GetD3DBlend(dst));
		m_Device->SetRenderState(D3DRS_SRCBLEND, GetD3DBlend(src));
		m_Device->SetRenderState(D3DRS_BLENDOP, GetD3DBlendFunc(op));
	}

	m_SrcBlendFactor = src;
	m_DstBlendFactor = dst;
	m_BlendOperator = op;
}

void DeviceStateD3D9::EnableVertexData(bool useColor)
{
	if(m_UseVertexData == useColor && !m_ResetAll)
		return;

	SetRenderState(D3DRS_COLORVERTEX, useColor ? TRUE : FALSE);
	m_UseVertexData = useColor;
}

void* DeviceStateD3D9::GetLowLevelDevice()
{
	return m_Device;
}

void DeviceStateD3D9::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	m_Device->SetRenderState(state, value);
}

void DeviceStateD3D9::SetRenderStateF(D3DRENDERSTATETYPE state, float value)
{
	DWORD v;
	memcpy(&v, &value, 4);
	SetRenderState(state, v);
}

void DeviceStateD3D9::SetTextureStageState(u32 stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	m_Device->SetTextureStageState(stage, state, value);
}

void DeviceStateD3D9::SetTexture(u32 stage, IDirect3DBaseTexture9* tex)
{
	if(stage >= m_Textures.Size())
		m_Textures.Resize(stage + 1, nullptr);

	if(tex != m_Textures[stage] || m_ResetAll) {
		HRESULT hr = m_Device->SetTexture(stage, tex);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);

		m_Textures[stage] = tex;
	}
}

void DeviceStateD3D9::SetTransform(D3DTRANSFORMSTATETYPE type, const math::Matrix4& m)
{
	m_Device->SetTransform(type, (D3DMATRIX*)m.DataRowMajor());
}

u32 DeviceStateD3D9::GetFillMode(const Pass& p)
{
	switch(p.drawMode) {
	case EDrawMode::Fill:
		return D3DFILL_SOLID;
	case EDrawMode::Wire:
		return D3DFILL_WIREFRAME;
	case EDrawMode::Point:
		return D3DFILL_POINT;
	default: throw core::InvalidArgumentException("pipeline");
	}
}

u32 DeviceStateD3D9::GetCullMode(const Pass& p)
{
	if(p.backfaceCulling)
		return D3DCULL_CCW;
	else if(p.frontfaceCulling)
		return D3DCULL_CW;
	else
		return D3DCULL_NONE;
}

u32 DeviceStateD3D9::Float2U32(float f)
{
	u32 out;
	static_assert(sizeof(out) == sizeof(f), "Float isn't 4 byte big");
	memcpy(&out, &f, 4);

	return out;
}

u32 DeviceStateD3D9::GetTextureOperator(ETextureOperator op)
{
	switch(op) {
	case ETextureOperator::Disable:
		return D3DTOP_DISABLE;
	case ETextureOperator::SelectArg1:
		return D3DTOP_SELECTARG1;
	case ETextureOperator::SelectArg2:
		return D3DTOP_SELECTARG2;
	case ETextureOperator::Modulate:
		return D3DTOP_MODULATE;
	case ETextureOperator::Add:
		return D3DTOP_ADD;
	case ETextureOperator::AddSigned:
		return D3DTOP_ADDSIGNED;
	case ETextureOperator::AddSmoth:
		return D3DTOP_ADDSMOOTH;
	case ETextureOperator::Subtract:
		return D3DTOP_SUBTRACT;
	case ETextureOperator::Blend:
		return D3DTOP_BLENDDIFFUSEALPHA;
	case ETextureOperator::Dot:
		return D3DTOP_DOTPRODUCT3;
	}
	throw core::InvalidArgumentException("operator");
}

u32 DeviceStateD3D9::GetTextureArgument(ETextureArgument arg)
{
	switch(arg) {
	case ETextureArgument::Current:
		return D3DTA_CURRENT;
	case ETextureArgument::Texture:
		return D3DTA_TEXTURE;
	case ETextureArgument::Diffuse:
		return D3DTA_DIFFUSE;
	case ETextureArgument::AlphaRep:
		return D3DTA_ALPHAREPLICATE;
	}
	throw core::InvalidArgumentException("arg");
}

void DeviceStateD3D9::EnableFog(bool enable)
{
	SetRenderState(D3DRS_FOGENABLE, enable ? TRUE : FALSE);
}

void DeviceStateD3D9::SetFog(const FogData& fog)
{
	DWORD type = GetD3DFogType(fog.type);

	// TODO: Handle per pixel fog
	SetRenderState(D3DRS_FOGVERTEXMODE, type);
	SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);

	SetRenderState(D3DRS_FOGCOLOR, (u32)ColorFToColor(fog.color));
	SetRenderStateF(D3DRS_FOGSTART, fog.start);
	SetRenderStateF(D3DRS_FOGEND, fog.end);
	SetRenderStateF(D3DRS_FOGDENSITY, fog.density);
}

void DeviceStateD3D9::EnableLight(bool enable)
{
	SetRenderState(D3DRS_LIGHTING, enable ? TRUE : FALSE);
	m_UseLighting = enable;
}

void DeviceStateD3D9::ClearLights()
{
	for(size_t i = 0; i < m_LightCount; ++i)
		m_Device->LightEnable((DWORD)i, FALSE);
	m_LightCount = 0;
}

void DeviceStateD3D9::AddLight(const LightData& light, ELighting lighting)
{
	DWORD lightId = (DWORD)m_LightCount;

	D3DLIGHT9 D3DLight;

	switch(light.type) {
	case ELightType::Point:
		D3DLight.Type = D3DLIGHT_POINT;
		break;
	case ELightType::Spot:
		D3DLight.Type = D3DLIGHT_SPOT;
		break;
	case ELightType::Directional:
		D3DLight.Type = D3DLIGHT_DIRECTIONAL;
		break;
	}

	D3DLight.Position = *((D3DVECTOR*)(&light.position));
	D3DLight.Direction = *((D3DVECTOR*)(&light.direction));

	D3DLight.Range = math::Max(0.0f, light.range);
	D3DLight.Falloff = light.falloff;

	D3DCOLORVALUE specular = {1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE ambient = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DCOLORVALUE black = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DLight.Diffuse = TestFlag(lighting, ELighting::Diffuse) ? SColorToD3DColor(light.color) : black;
	D3DLight.Specular = TestFlag(lighting, ELighting::Specular) ? SColorToD3DColor(light.color) : black;
	D3DLight.Ambient = TestFlag(lighting, ELighting::Ambient) ? ambient : black;

	D3DLight.Attenuation0 = 0.0f;
	D3DLight.Attenuation1 = 0.0f;
	D3DLight.Attenuation2 = 1.0f / light.range;

	D3DLight.Theta = light.innerCone * 2.0f;
	D3DLight.Phi = light.outerCone * 2.0f;

	HRESULT hr;
	if(FAILED(hr = m_Device->SetLight(lightId, &D3DLight)))
		throw core::D3D9Exception(hr);

	if(FAILED(hr = m_Device->LightEnable(lightId, TRUE)))
		throw core::D3D9Exception(hr);

	++m_LightCount;
}

void DeviceStateD3D9::SetRenderTargetTexture(video::BaseTexture* t)
{
	m_RenderTargetTexture = t;
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
