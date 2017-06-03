#ifdef LUX_COMPILE_WITH_D3D9
#include "DeviceStateD3D9.h"

#include "video/d3d9/D3DHelper.h"
#include "video/d3d9/D3D9Exception.h"

#include "video/PipelineSettings.h"
#include "video/AlphaSettings.h"

#include "video/FogData.h"
#include "video/LightData.h"
#include "video/Material.h"

#include "video/BaseTexture.h"

namespace lux
{
namespace video
{

DeviceStateD3D9::DeviceStateD3D9(IDirect3DDevice9* device) :
	m_Device(device),
	m_RenderTargetTexture(nullptr),
	m_CurShader(nullptr),
	m_LightCount(0)
{
	SetRenderState(D3DRS_COLORVERTEX, FALSE);
}

void DeviceStateD3D9::EnablePipeline(const PipelineSettings& pipeline)
{
	// These are handled elsewhere
	/*
	pipeline.Lightning
	pipeling.FogEnabled
	*/
	m_Device->SetRenderState(D3DRS_ZFUNC, GetD3DZBufferFunc(pipeline.zBufferFunc));
	m_Device->SetRenderState(D3DRS_ZWRITEENABLE, pipeline.zWriteEnabled ? TRUE : FALSE);
	m_Device->SetRenderState(D3DRS_NORMALIZENORMALS, pipeline.normalizeNormals ? TRUE : FALSE);
	m_Device->SetRenderState(D3DRS_FILLMODE, GetFillMode(pipeline));
	m_Device->SetRenderState(D3DRS_SHADEMODE, pipeline.gouraudShading ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
	m_Device->SetRenderState(D3DRS_CULLMODE, GetCullMode(pipeline));
}

void DeviceStateD3D9::EnableTextureLayer(u32 stage, const TextureLayer& layer)
{
	HRESULT hr;
	if(layer.texture) {
		// The current rendertarget can't be used as texture -> set texture channel to black
		// But tell all people asking that the correct texture was set.
		if(layer.texture == m_RenderTargetTexture)
			hr = m_Device->SetTexture(stage, nullptr);
		else
			hr = m_Device->SetTexture(stage, (IDirect3DBaseTexture9*)(layer.texture->GetRealTexture()));
	} else {
		hr = m_Device->SetTexture(stage, nullptr);
	}

	if(layer.texture) {
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

void DeviceStateD3D9::EnableAlpha(const AlphaBlendSettings& settings)
{
	if(settings.Operator == EBlendOperator::None) {
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	} else {
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_Device->SetRenderState(D3DRS_DESTBLEND, GetD3DBlend(settings.DstBlend));
		m_Device->SetRenderState(D3DRS_SRCBLEND, GetD3DBlend(settings.SrcBlend));
		m_Device->SetRenderState(D3DRS_BLENDOP, GetD3DBlendFunc(settings.Operator));
	}
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

void DeviceStateD3D9::SetD3DMaterial(const video::Colorf& ambient, const PipelineSettings& pipeline, const video::Material* mat)
{
	D3DMATERIAL9 D3DMaterial = {
		SColorToD3DColor(mat->GetDiffuse()),
		SColorToD3DColor(mat->GetDiffuse()*mat->GetAmbient()),
		SColorToD3DColor(mat->GetSpecular()),
		SColorToD3DColor(mat->GetEmissive()),
		mat->GetShininess()
	};
	m_Device->SetMaterial(&D3DMaterial);

	SetRenderState(D3DRS_AMBIENT, ambient.ToHex());
	if(pipeline.lighting && !math::IsZero(D3DMaterial.Power))
		SetRenderState(D3DRS_SPECULARENABLE, 1);
	else
		SetRenderState(D3DRS_SPECULARENABLE, 0);
}

void DeviceStateD3D9::SetTransform(D3DTRANSFORMSTATETYPE type, const math::matrix4& m)
{
	m_Device->SetTransform(type, (D3DMATRIX*)m.DataRowMajor());
}

u32 DeviceStateD3D9::GetFillMode(const PipelineSettings& pipeline)
{
	switch(pipeline.drawMode) {
	case EDrawMode::Fill:
		return D3DFILL_SOLID;
	case EDrawMode::Wire:
		return D3DFILL_WIREFRAME;
	case EDrawMode::Point:
		return D3DFILL_POINT;
	default: throw core::InvalidArgumentException("pipeline");
	}
}

u32 DeviceStateD3D9::GetCullMode(const PipelineSettings& pipeline)
{
	if(pipeline.backfaceCulling)
		return D3DCULL_CCW;
	else if(pipeline.frontfaceCulling)
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

void DeviceStateD3D9::SetFog(bool active, const FogData& fog)
{
	if(!active) {
		SetRenderState(D3DRS_FOGENABLE, FALSE);
		return;
	}

	SetRenderState(D3DRS_FOGENABLE, TRUE);

	DWORD type = GetD3DFogType(fog.type);

	// TODO: Handle per pixel fog
	SetRenderState(D3DRS_FOGVERTEXMODE, type);
	SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);

	SetRenderState(D3DRS_FOGCOLOR, (u32)ColorFToColor(fog.color));
	SetRenderStateF(D3DRS_FOGSTART, fog.start);
	SetRenderStateF(D3DRS_FOGEND, fog.end);
	SetRenderStateF(D3DRS_FOGDENSITY, fog.density);
}

void DeviceStateD3D9::ClearLights(bool useLights)
{
	for(size_t i = 0; i < m_LightCount; ++i)
		m_Device->LightEnable((DWORD)i, FALSE);

	SetRenderState(D3DRS_LIGHTING, useLights ? 1 : 0);
}

void DeviceStateD3D9::EnableLight(const LightData& light)
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

	D3DCOLORVALUE Specular = {1.0f, 1.0f, 1.0f, 1.0f};
	D3DCOLORVALUE Ambient = {0.0f, 0.0f, 0.0f, 0.0f};
	D3DLight.Diffuse = SColorToD3DColor(light.color);
	D3DLight.Specular = Specular;
	D3DLight.Ambient = Ambient;

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
