#ifndef INCLUDED_CMATERIALRENDERER_H
#define INCLUDED_CMATERIALRENDERER_H
#include "video/MaterialRenderer.h"
#include "video/Material.h"
#include "video/VideoDriver.h"
#include "StrippedD3D9.h"

namespace lux
{
namespace video
{

class MaterialRendererD3D9 : public MaterialRenderer
{
public:
	MaterialRendererD3D9(VideoDriver* p, Shader* pShader = nullptr, const core::ParamPackage* BasePackage = nullptr) : MaterialRenderer(p, pShader, BasePackage), m_D3DDevice((IDirect3DDevice9*)p->GetDevice())
	{
	}

	virtual ~MaterialRendererD3D9()
	{
	}

protected:
	D3DCOLORVALUE SColorToD3DColor(const Colorf& color)
	{
		D3DCOLORVALUE out = {color.r,
			color.g,
			color.b,
			color.a};

		return out;
	}

	u32 GetD3DBlend(EBlendFactor factor)
	{
		switch(factor) {
		case EBF_ZERO:
			return D3DBLEND_ZERO;
		case EBF_ONE:
			return D3DBLEND_ONE;
		case EBF_SRC_ALPHA:
			return D3DBLEND_SRCALPHA;
		case EBF_ONE_MINUS_SRC_ALPHA:
			return D3DBLEND_INVSRCALPHA;
		case EBF_DST_ALPHA:
			return D3DBLEND_DESTALPHA;
		case EBF_ONE_MINUS_DST_ALPHA:
			return D3DBLEND_INVDESTALPHA;
		default:
			return 0;
		}
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
		m_Package.AddParam("DiffMap", MaterialLayer());
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		m_Pipeline.Blending.Operator = EBO_NONE;
		Enable(material.GetPuffer(), resetAll);

		// Textur setzen
		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);

		if(material.GetRenderer() != lastMaterial.GetRenderer() || resetAll) {
			// Nur eine Texturschicht
			// Texturfarbe und Vertexfarbe durch Multiplikation verknüpfen
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

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
		m_Package.AddParam<u32>("BlendFunc", Pack_TextureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA));
		m_Package.AddParam("DiffMap", MaterialLayer());

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

		Unpack_TextureBlendFunc(SrcFact, DstFact, Operator, AlphaSource, Value, NewParam);

		m_Pipeline.Blending.Operator = Operator;
		m_Pipeline.Blending.DstBlend = DstFact;
		m_Pipeline.Blending.SrcBlend = SrcFact;
		Enable(material.GetPuffer(), resetAll);

		m_Driver->SetTextureLayer(material.Layer(0), 0, resetAll);

		if(material.GetRenderer() != lastMaterial.GetRenderer() ||
			NewParam != LastParam || resetAll) {

			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			if(TextureBlendFunc_HasAlpha(SrcFact) || TextureBlendFunc_HasAlpha(DstFact)) {
				if(AlphaSource == EAS_VERTEX_COLOR) {
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				} else if(AlphaSource == EAS_VERTEX_AND_TEXTURE || (AlphaSource == EAS_TEXTURE && material.diffuse.HasAlpha())) {
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
					m_D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				} else if(AlphaSource == EAS_TEXTURE) {
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
		m_Package.AddParam("DiffMap", MaterialLayer());
		m_Pipeline.ZWriteEnabled = false;
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		// Basis-Renderstates einsetzen
		//m_pDriver->SetBasicRenderstates(material, lastMaterial, bResetAll);
		Enable(material.GetPuffer(), resetAll);

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
		m_Package.AddParam("DiffMap1", MaterialLayer());
		m_Package.AddParam("DiffMap2", MaterialLayer());
	}

	void OnSetMaterial(const Material& material, const Material& lastMaterial, bool resetAll = false)
	{
		m_Pipeline.Blending.Operator = EBO_NONE;
		Enable(material.GetPuffer(), resetAll);

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

			m_D3DDevice->SetTextureStageState(2, D3DTSS_COLOROP,  D3DTOP_MODULATE);
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
		m_D3DDevice->SetTextureStageState(2, D3DTSS_COLOROP,  D3DTOP_DISABLE);
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

}    // namespace video
}    // namespace lux

#endif