#ifndef INCLUDED_MATERIALRENDERER_D3D9_H
#define INCLUDED_MATERIALRENDERER_D3D9_H
#include "video/MaterialRenderer.h"
#include "video/Material.h"
#include "video/VideoDriver.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"
#include "video/d3d9/D3DHelper.h"

namespace lux
{
namespace video
{

class MaterialRendererD3D9 : public MaterialRenderer
{
public:
	MaterialRendererD3D9(VideoDriver* p,
		Shader* pShader = nullptr,
		const core::ParamPackage* BasePackage = nullptr) :
		MaterialRenderer(p, pShader, BasePackage),
		m_D3DDevice((IDirect3DDevice9*)p->GetDevice())
	{
	}

protected:
	IDirect3DDevice9* m_D3DDevice;
};

// Rendert ein opakes material
class MaterialRenderer_Solid_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_Solid_d3d9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererD3D9(p, pShader, BasePackage)
	{
		m_Package.AddParam("DiffMap", TextureLayer());
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		m_Pipeline.Blending.Operator = EBlendOperator::None;
		Enable(material.GetPuffer(), resetAll, &material);

		// Textur setzen
		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);

		// Nur eine Texturschicht
		// Texturfarbe und Vertexfarbe durch Multiplikation verknüpfen
		if((BaseTexture*)material.Layer(0) == nullptr) {
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

		} else {
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		}

		if(material.GetRenderer() != lastMaterial.GetRenderer() || resetAll) {
			m_D3DDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
		}

		// Zweite Texturschicht deaktivieren
		m_D3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}

	MaterialRenderer* Clone(Shader* pShader, const core::ParamPackage* BasePackage) const
	{
		MaterialRenderer_Solid_d3d9* renderer = LUX_NEW(MaterialRenderer_Solid_d3d9)(m_Driver, pShader, BasePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

// Rendert ein transparentes material mit einer Textur(Alpha aus Textur, Farbe)
class MaterialRenderer_OneTextureBlend_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_OneTextureBlend_d3d9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererD3D9(p, pShader, BasePackage)
	{
		m_Package.AddParam<u32>("BlendFunc", PackTextureBlendFunc(video::EBlendFactor::SrcAlpha, video::EBlendFactor::OneMinusSrcAlpha));
		m_Package.AddParam("DiffMap", TextureLayer());

		m_Pipeline.ZWriteEnabled = false;
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		// Basis-Renderstates einsetzen
		// Blendinfos entpacken
		EBlendFactor SrcFact;
		EBlendFactor DstFact;
		EBlendOperator Operator;
		EAlphaSource AlphaSource;
		u32 Value;

		u32 NewParam = 0, LastParam = 0;
		NewParam = material.Param(0);
		if(material.GetRenderer() == lastMaterial.GetRenderer()) {
			LastParam = lastMaterial.Param(0);
		}

		UnpackTextureBlendFunc(SrcFact, DstFact, Operator, AlphaSource, Value, NewParam);

		m_Pipeline.Blending.Operator = Operator;
		m_Pipeline.Blending.DstBlend = DstFact;
		m_Pipeline.Blending.SrcBlend = SrcFact;
		Enable(material.GetPuffer(), resetAll, &material);

		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);

		if(material.GetRenderer() != lastMaterial.GetRenderer() ||
			NewParam != LastParam || resetAll) {

			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			if(HasTextureBlendAlpha(SrcFact) || HasTextureBlendAlpha(DstFact)) {
				if(AlphaSource == EAlphaSource::VertexColor) {
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				} else if(AlphaSource == EAlphaSource::VertexAndTexture || (AlphaSource == EAlphaSource::Texture && material.diffuse.HasAlpha())) {
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				} else if(AlphaSource == EAlphaSource::Texture) {
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				}
			}

			// Zweite Texturschicht deaktivieren
			m_D3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}
	}

	bool IsTransparent() const
	{
		return true;
	}

	MaterialRenderer* Clone(Shader* pShader, const core::ParamPackage* BasePackage) const
	{
		MaterialRenderer_OneTextureBlend_d3d9* renderer = LUX_NEW(MaterialRenderer_OneTextureBlend_d3d9)(m_Driver, pShader, BasePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

// Rendert ein transparentes material mit dem Alpha aus Vertexfarbe
class CMaterialRenderer_VertexAlpha_d3d9 : public MaterialRendererD3D9
{
public:
	CMaterialRenderer_VertexAlpha_d3d9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererD3D9(p, pShader, BasePackage)
	{
		m_Package.AddParam<float>("AlphaFactor", 1.0f);
		m_Package.AddParam("DiffMap", TextureLayer());
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

class MaterialRenderer_Solid_Mix_d3d9 : public MaterialRendererD3D9
{
public:
	MaterialRenderer_Solid_Mix_d3d9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRendererD3D9(p, pShader, BasePackage)
	{
		m_Package.AddParam("DiffMap1", TextureLayer());
		m_Package.AddParam("DiffMap2", TextureLayer());
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		m_Pipeline.Blending.Operator = EBlendOperator::None;
		Enable(material.GetPuffer(), resetAll, &material);

		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);
		m_Driver->SetTextureLayer(material.Layer(1), 1, resetAll);

		if(material.GetRenderer() != lastMaterial.GetRenderer() || resetAll) {
			// Blend(Diff*Texture1, Texture2, DiffAlpha)
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			//m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

			m_D3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
			m_D3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			m_D3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);

			m_D3DDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_D3DDevice->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
			m_D3DDevice->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			// Use the same texture-coordinates as the first layer
			m_D3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			m_D3DDevice->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0);

			m_D3DDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		}
	}

	virtual void OnUnsetMaterial()
	{
		m_D3DDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
		m_D3DDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}

	bool IsTransparent() const
	{
		return false;
	}


	MaterialRenderer* Clone(Shader* pShader, const core::ParamPackage* BasePackage) const
	{
		MaterialRenderer_Solid_Mix_d3d9* renderer = LUX_NEW(MaterialRenderer_Solid_Mix_d3d9)(m_Driver, pShader, BasePackage);
		renderer->m_Pipeline = m_Pipeline;
		return renderer;
	}
};

}

}


#endif // LUX_COMPILE_WITH_D3D9

#endif