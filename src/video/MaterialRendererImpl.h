#ifndef INCLUDED_MATERIALRENDERER_IMPL_H
#define INCLUDED_MATERIALRENDERER_IMPL_H
#include "video/MaterialRenderer.h"
#include "video/MaterialImpl.h"

#include "video/RenderSettings.h"
#include "video/TextureStageSettings.h"
#include "video/AlphaSettings.h"
#include "video/PipelineSettings.h"

#include "video/Renderer.h"
#include "video/DeviceState.h"
#include "video/Shader.h"

#include "core/ParamPackage.h"

namespace lux
{
namespace video
{

//! The base class for material renderers
class MaterialRendererImpl : public MaterialRenderer
{
public:
	MaterialRendererImpl(const string& name, Shader* shader) :
		m_Shader(shader),
		m_Name(name)
	{
	}

	virtual void Begin(const RenderSettings& settings, DeviceState& state)
	{
		EnableTextures(settings, state);

		if(m_Shader)
			state.EnableShader(m_Shader);
		else
			state.DisableCurShader();
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

	virtual core::ParamPackage& GetPackage()
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

	const string& GetName() const
	{
		return m_Name;
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

	string m_Name;
};

class MaterialRenderer_DebugOverlay : public MaterialRendererImpl
{
public:
	MaterialRenderer_DebugOverlay(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		m_Pipeline.fogEnabled = false;
		m_Pipeline.lighting = false;

		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);

		state.EnableVertexData();
		TextureStageSettings tss;
		tss.colorOperator = ETextureOperator::SelectArg1;
		tss.colorArg1 = ETextureArgument::Diffuse;

		state.EnableTextureStage(0, tss);
	}

	void End(DeviceState& state)
	{
		state.DisableVertexData();
		state.DisableTextureStage(0);

		MaterialRendererImpl::End(state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_DebugOverlay> renderer = LUX_NEW(MaterialRenderer_DebugOverlay)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

class MaterialRenderer_BaseSolid : public MaterialRendererImpl
{
public:
	MaterialRenderer_BaseSolid(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		if(basePackage)
			m_Package = *basePackage;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_BaseSolid> renderer = LUX_NEW(MaterialRenderer_BaseSolid)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

class MaterialRenderer_BaseTransparent : public MaterialRendererImpl
{
public:
	MaterialRenderer_BaseTransparent(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		AlphaBlendSettings defaultSettings;
		defaultSettings.SrcBlend = video::EBlendFactor::SrcAlpha;
		defaultSettings.DstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		defaultSettings.Operator = video::EBlendOperator::Add;
		m_Package.AddParam<u32>("blendFunc", defaultSettings.Pack());

		if(basePackage)
			m_Package.MergePackage(*basePackage);

		m_Pipeline.zWriteEnabled = false;
		m_Pipeline.fogEnabled = false;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);

		AlphaBlendSettings blend;
		blend.Unpack(settings.material->Param(0));

		state.EnableAlpha(blend);
	}

	void End(DeviceState& state)
	{
		state.EnableAlpha(video::AlphaBlendSettings::Disabled());
		MaterialRendererImpl::End(state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::Transparent;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_BaseTransparent> renderer = LUX_NEW(MaterialRenderer_BaseTransparent)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

//! Render a solid/oname, pak material
class MaterialRenderer_Solid : public MaterialRendererImpl
{
public:
	MaterialRenderer_Solid(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		m_Package.AddParam("diffMap", TextureLayer());
		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);

		TextureStageSettings tss0;
		if((BaseTexture*)settings.material->Layer(0)) {
			tss0.colorOperator = ETextureOperator::Modulate;
			tss0.colorArg1 = ETextureArgument::Texture;
			tss0.colorArg2 = ETextureArgument::Diffuse;
		} else {
			tss0.colorOperator = ETextureOperator::SelectArg1;
			tss0.colorArg1 = ETextureArgument::Diffuse;
		}

		tss0.useVertex = settings.pipeline.useVertex;
		state.EnableTextureStage(0, tss0);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_Solid> renderer = LUX_NEW(MaterialRenderer_Solid)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

//! Render a transparent material with a single texture, alpha from texture or color
class MaterialRenderer_OneTextureBlend : public MaterialRendererImpl
{
public:
	MaterialRenderer_OneTextureBlend(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		AlphaBlendSettings defaultSettings;
		defaultSettings.SrcBlend = video::EBlendFactor::SrcAlpha;
		defaultSettings.DstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		defaultSettings.Operator = video::EBlendOperator::Add;
		m_Package.AddParam<u32>("blendFunc", defaultSettings.Pack());
		m_Package.AddParam("diffMap", TextureLayer());

		if(basePackage)
			m_Package.MergePackage(*basePackage);

		m_Pipeline.zWriteEnabled = false;
		m_Pipeline.fogEnabled = false;
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);

		AlphaBlendSettings blend;
		blend.Unpack(settings.material->Param(0));

		state.EnableAlpha(blend);

		TextureStageSettings tss0;
		if((BaseTexture*)settings.material->Layer(0)) {
			tss0.colorOperator = ETextureOperator::Modulate;
			tss0.colorArg1 = ETextureArgument::Texture;
			tss0.colorArg2 = ETextureArgument::Diffuse;
		} else {
			tss0.colorOperator = ETextureOperator::SelectArg1;
			tss0.colorArg1 = ETextureArgument::Diffuse;
		}

		tss0.useVertex = settings.pipeline.useVertex;
		state.EnableTextureStage(0, tss0);
	}

	void End(DeviceState& state)
	{
		state.DisableAlpha();
		MaterialRendererImpl::End(state);
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::Transparent;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_OneTextureBlend> renderer = LUX_NEW(MaterialRenderer_OneTextureBlend)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

/*
//! Render a transparent material with a single texture, alpha from vertex
class CMaterialRenderer_VertexAlpha : public MaterialRendererImpl
{
public:
	CMaterialRenderer_VertexAlpha(const string& name, VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererImpl(p, pShader, BasePackagename, )
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

	MaterialRenderer* Clone(const string& name, Shader* pShader, const core::ParamPackage* BasePackage) const
	{
		if(!pShader)
			return nullptr;

		CMname, aterialRenderer_VertexAlpha* renderer = LUX_NEW(CMaterialRenderer_VertexAlpha)(m_Driver, pShader, BasePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};
*/

//! Render a solid material, with two blended textures
/**
The alpha color of the vertices determinates the interpolation value
*/
class MaterialRenderer_Solid_Mix : public MaterialRendererImpl
{
public:
	MaterialRenderer_Solid_Mix(const string& name, Shader* shader, const core::ParamPackage* basePackage) :
		MaterialRendererImpl(name, shader)
	{
		m_Package.AddParam("diffMap1", TextureLayer());
		m_Package.AddParam("diffMap2", TextureLayer());

		if(basePackage)
			m_Package.MergePackage(*basePackage);
	}

	void Begin(const RenderSettings& settings, DeviceState& state)
	{
		MaterialRendererImpl::Begin(settings, state);

		state.EnableVertexData();

		// Blend(Diff*Texture1, Texture2, DiffAlpha)
		TextureStageSettings tss0, tss1, tss2;
		tss0.colorOperator = ETextureOperator::SelectArg1;
		tss0.colorArg1 = ETextureArgument::Texture;

		tss1.colorOperator = ETextureOperator::Blend;
		tss1.colorArg1 = ETextureArgument::Current;
		tss1.colorArg2 = ETextureArgument::Texture;
		tss1.coordSource = 0;

		tss2.colorOperator = ETextureOperator::Modulate;
		tss2.colorArg1 = ETextureArgument::Current;
		tss2.colorArg2 = ETextureArgument::Diffuse;
		tss2.coordSource = 0;

		state.EnableTextureStage(0, tss0);
		state.EnableTextureStage(1, tss1);
		state.EnableTextureStage(2, tss2);
	}

	void End(DeviceState& state)
	{
		state.DisableTextureStage(0);
		state.DisableTextureStage(1);
		state.DisableTextureStage(2);
		state.DisableVertexData();
	}

	ERequirement GetRequirements() const
	{
		return ERequirement::None;
	}

	StrongRef<MaterialRenderer> Clone(const string& name, Shader* shader, const core::ParamPackage* basePackage) const
	{
		StrongRef<MaterialRenderer_Solid_Mix> renderer = LUX_NEW(MaterialRenderer_Solid_Mix)(name, shader, basePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

} // namespace video
} // namespace lux

#endif