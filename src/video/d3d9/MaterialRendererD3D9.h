#ifndef INCLUDED_MATERIALRENDERER_D3D9_H
#define INCLUDED_MATERIALRENDERER_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/MaterialRenderer.h"
#include "video/MaterialImpl.h"
#include "video/RenderSettings.h"
#include "video/Renderer.h"
#include "video/DeviceState.h"
#include "video/Shader.h"

#include "core/ParamPackage.h"

#include "StrippedD3D9.h"
#include "video/d3d9/D3DHelper.h"

namespace lux
{
namespace video
{

//! The base class for d3d9 material renderers
class MaterialRendererD3D9 : public MaterialRenderer
{
public:
	MaterialRendererD3D9(Shader* shader) :
		m_Shader(shader)
	{
	}

	virtual void Begin(const RenderSettings& settings, DeviceState& state)
	{
		EnableTextures(settings, state);

		if(m_Shader) {
			state.EnableShader(m_Shader);
		} else {
			state.DisableCurShader();
		}
	}

	virtual void End(DeviceState& state)
	{
		LUX_UNUSED(state);
	}

	virtual const PipelineSettings& GetPipeline() const
	{
		return m_Pipeline;
	}

	virtual void SetPipeline(const PipelineSettings& set)
	{
		m_Pipeline = set;
	}

	virtual const core::ParamPackage& GetPackage() const
	{
		return m_Package;
	}

	virtual StrongRef<Shader> GetShader() const
	{
		return m_Shader;
	}

	virtual StrongRef<Material> CreateMaterial()
	{
		return LUX_NEW(MaterialImpl)(this);
	}

private:
	u32 EnableTextures(const RenderSettings& settings, DeviceState& state)
	{
		u32 curStage = 0;
		for(size_t i = 0; i < settings.material->GetTextureCount(); ++i) {
			state.EnableTextureLayer(curStage, settings.material->Layer(i));
			++curStage;
		}

		if(m_Shader) {
			for(size_t i = 0; i < m_Shader->GetSceneParamCount(); ++i) {
				u32 id = m_Shader->GetSceneParam(i);
				if(settings.params[id].GetType() == core::Type::Texture) {
					state.EnableTextureLayer(curStage, settings.params[id]);
					++curStage;
				}
			}
		}

		return curStage;
	}

protected:
	StrongRef<Shader> m_Shader;

	core::ParamPackage m_Package;
	PipelineSettings m_Pipeline;
};

class MaterialRenderer_DebugOverlay_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_DebugOverlay_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		m_Pipeline.fogEnabled = false;
		m_Pipeline.lighting = false;

		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);
		auto device = (IDirect3DDevice9*)state.GetLowLevelDevice();

		device->SetRenderState(D3DRS_TEXTUREFACTOR, settings.material->GetDiffuse().ToHex());
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_DebugOverlay_d3d9> renderer = LUX_NEW(MaterialRenderer_DebugOverlay_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

class MaterialRenderer_BaseSolid_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_BaseSolid_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		if(basePackage)
			m_Package = *basePackage;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_BaseSolid_d3d9> renderer = LUX_NEW(MaterialRenderer_BaseSolid_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

class MaterialRenderer_BaseTransparent_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_BaseTransparent_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		AlphaBlendSettings defaultSettings;
		defaultSettings.SrcBlend = video::EBlendFactor::SrcAlpha;
		defaultSettings.DstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		defaultSettings.Operator = video::EBlendOperator::Add;
		defaultSettings.AlphaSrc = video::EAlphaSource::Texture;
		m_Package.AddParam<u32>("blendFunc", defaultSettings.Pack());

		if(basePackage)
			m_Package.MergePackage(*basePackage);

		m_Pipeline.zWriteEnabled = false;
		m_Pipeline.fogEnabled = false;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);

		AlphaBlendSettings blend;
		blend.Unpack(settings.material->Param(0));

		state.EnableAlpha(blend);
	}

	void End(DeviceState& state)
	{
		state.EnableAlpha(video::AlphaBlendSettings::Disabled());
		MaterialRendererD3D9::End(state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::Transparent;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_BaseTransparent_d3d9> renderer = LUX_NEW(MaterialRenderer_BaseTransparent_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

//! Render a solid/opak material
class MaterialRenderer_Solid_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_Solid_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		m_Package.AddParam("diffMap", TextureLayer());
		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);
		auto device = (IDirect3DDevice9*)state.GetLowLevelDevice();

		BaseTexture* firstLayer = settings.material->Layer(0);

		if(!settings.pipeline.lighting) {
			device->SetRenderState(D3DRS_TEXTUREFACTOR, settings.material->GetDiffuse().ToHex());
			device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		} else {
			device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		}

		if(firstLayer) {
			device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		} else {
			// Use only the diffuse color
			device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
		}
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_Solid_d3d9> renderer = LUX_NEW(MaterialRenderer_Solid_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

//! Render a transparent material with a single texture, alpha from texture or color
class MaterialRenderer_OneTextureBlend_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_OneTextureBlend_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		AlphaBlendSettings defaultSettings;
		defaultSettings.SrcBlend = video::EBlendFactor::SrcAlpha;
		defaultSettings.DstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		defaultSettings.Operator = video::EBlendOperator::Add;
		defaultSettings.AlphaSrc = video::EAlphaSource::Texture;
		m_Package.AddParam<u32>("blendFunc", defaultSettings.Pack());
		m_Package.AddParam("diffMap", TextureLayer());

		if(basePackage)
			m_Package.MergePackage(*basePackage);

		m_Pipeline.zWriteEnabled = false;
		m_Pipeline.fogEnabled = false;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);
		auto device = (IDirect3DDevice9*)state.GetLowLevelDevice();

		AlphaBlendSettings blend;
		blend.Unpack(settings.material->Param(0));

		state.EnableAlpha(blend);

		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		if(HasTextureBlendAlpha(blend.SrcBlend) || HasTextureBlendAlpha(blend.DstBlend)) {
			if(blend.AlphaSrc == EAlphaSource::VertexColor) {
				device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			} else if(blend.AlphaSrc == EAlphaSource::VertexAndTexture || (blend.AlphaSrc == EAlphaSource::Texture && settings.material->GetDiffuse().HasAlpha())) {
				device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			} else if(blend.AlphaSrc == EAlphaSource::Texture) {
				device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			}
		}
	}

	void End(DeviceState& state)
	{
		state.EnableAlpha(video::AlphaBlendSettings::Disabled());
		MaterialRendererD3D9::End(state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::Transparent;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_OneTextureBlend_d3d9> renderer = LUX_NEW(MaterialRenderer_OneTextureBlend_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

/*
//! Render a transparent material with a single texture, alpha from vertex
class CMaterialRenderer_VertexAlpha_d3d9 : public MaterialRendererD3D9
{
public:
	CMaterialRenderer_VertexAlpha_d3d9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererD3D9(p, pShader, BasePackage)
	{
		m_Package.AddParam<float>("AlphaFactor", 1.0f);
		m_Package.AddParam("diffMap", TextureLayer());
		m_Pipeline.ZWriteEnabled = false;
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		// Basis-Renderstates einsetzen
		//m_pDriver->SetBasicRenderstates(material, lastMaterial, bResetAll);
		Enable(material.GetPuffer(), resetAll, &material);

		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);

		if(material.GetRenderer() != lastMaterial.GetRenderer() || resetAll) {
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

			m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

			float AlphaFactor = material.Param(0);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			u32 Alpha = (u32)(AlphaFactor*255.0f);
			m_D3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(Alpha, 255, 255, 255));

			m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			m_D3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}
	}

	bool IsTransparent() const
	{
		return true;
	}

	MaterialRenderer* Clone(Shader* pShader, const core::ParamPackage* BasePackage) const
	{
		if(!pShader)
			return nullptr;

		CMaterialRenderer_VertexAlpha_d3d9* renderer = LUX_NEW(CMaterialRenderer_VertexAlpha_d3d9)(m_Driver, pShader, BasePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};
*/

//! Render a solid material, with two blended textures
/**
The alpha color of the vertices determinates the interpolation value
*/
class MaterialRenderer_Solid_Mix_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_Solid_Mix_d3d9(Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererD3D9(shader)
	{
		m_Package.AddParam("diffMap1", TextureLayer());
		m_Package.AddParam("diffMap2", TextureLayer());

		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererD3D9::Begin(settings, state);

		auto device = (IDirect3DDevice9*)state.GetLowLevelDevice();

		// Blend(Diff*Texture1, Texture2, DiffAlpha)
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		//device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

		device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
		device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);

		device->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
		device->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		// Use the same texture-coordinates as the first layer
		device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
		device->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0);

		device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
	}

	void End(DeviceState& state)
	{
		auto device = (IDirect3DDevice9*)state.GetLowLevelDevice();

		device->SetRenderState(D3DRS_COLORVERTEX, FALSE);
		device->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_Solid_Mix_d3d9> renderer = LUX_NEW(MaterialRenderer_Solid_Mix_d3d9)(shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9
#endif