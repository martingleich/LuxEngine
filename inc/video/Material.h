#ifndef INCLUDED_LUX_MATERIAL_H
#define INCLUDED_LUX_MATERIAL_H
#include "video/AbstractMaterial.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

class Material : public AbstractMaterial
{
public:
	Material(const Pass& technique, EMaterialReqFlag req) :
		m_Technique(technique),
		m_Params(&technique.shader->GetParamPackage()),
		m_Requirements(req)
	{
		m_DiffuseId = m_Params.GetType()->GetParamIdByName("diffuse");
		m_EmissiveId = m_Params.GetType()->GetParamIdByName("emissive");
		m_SpecularHardnessId = m_Params.GetType()->GetParamIdByName("specularHardness");
		m_SpecularIntensityId = m_Params.GetType()->GetParamIdByName("specularIntensity");
	}

	const Pass& GetPass() const override
	{
		return m_Technique;
	}

	EMaterialReqFlag GetRequirements() const override { return m_Requirements; }

	void SendShaderSettings(const Pass& pass, void*) const override
	{
		for(int i = 0; i < m_Params.GetParamCount(); ++i) {
			auto param = m_Params.FromID(i, true);
			pass.shader->SetParam(i, param.Pointer());
		}
	}

	StrongRef<Material> Clone() const
	{
		return LUX_NEW(Material)(*this);
	}

	///////////////////////////////////

	core::VariableAccess Param(core::StringView name) const
	{
		return m_Params.FromName(name, true);
	}
	core::VariableAccess Param(core::StringView name)
	{
		return m_Params.FromName(name, false);
	}
	core::VariableAccess Param(int id) const
	{
		return m_Params.FromID(id, true);
	}
	core::VariableAccess Param(int id)
	{
		return m_Params.FromID(id, false);
	}

	void SetTexture(int layer, video::BaseTexture* texture)
	{
		auto type = m_Params.GetType();
		int l = 0;
		int i = 0;
		for(; i < type->GetParamCount(); ++i) {
			if(type->GetParamType(i) == core::Types::Texture()) {
				if(l == layer)
					break;
				++l;
			}
		}
		if(i != type->GetParamCount())
			m_Params.FromID(i, false) = video::TextureLayer(texture);
	}
	void SetTexture(core::StringView str, video::BaseTexture* texture)
	{
		m_Params.FromName(str, false) = video::TextureLayer(texture);
	}
	video::TextureLayer GetTexture(core::StringView str) const
	{
		return m_Params.FromName(str, true);
	}
	video::TextureLayer GetTexture(int id) const
	{
		return m_Params.FromID(id, true);
	}
	
	void SetColor(core::StringView str, const video::ColorF& color)
	{
		m_Params.FromName(str, false) = color;
	}
	void SetColor(int id, const video::ColorF& color)
	{
		m_Params.FromID(id, false) = color;
	}
	video::ColorF GetColor(core::StringView str) const
	{
		return m_Params.FromName(str, true);
	}
	video::ColorF GetColor(int id) const
	{
		return m_Params.FromID(id, true);
	}

	void SetFloat(core::StringView str, float f)
	{
		m_Params.FromName(str, false) = f;
	}
	void SetFloat(int id, float f)
	{
		m_Params.FromID(id, false) = f;
	}
	float GetFloat(core::StringView str) const
	{
		return m_Params.FromName(str, true);
	}
	float GetFloat(int id) const
	{
		return m_Params.FromID(id, true);
	}
	
	bool HasDiffuse() const { return m_DiffuseId > 0; }
	bool HasEmissive() const { return m_EmissiveId > 0; }
	bool HasSpecularHardness() const { return m_SpecularHardnessId > 0; }
	bool HasSpecularIntensity() const { return m_SpecularIntensityId > 0; }

	void SetDiffuse(const video::ColorF& color) { if(HasDiffuse()) SetColor(m_DiffuseId, color); }
	void SetEmissive(float emissive) { if(HasEmissive()) SetFloat(m_EmissiveId, emissive); }
	void SetSpecularHardness(float hardness) { if(HasSpecularHardness()) SetFloat(m_SpecularHardnessId, hardness); }
	void SetSpecularIntensity(float intensity) { if(HasSpecularIntensity()) SetFloat(m_SpecularIntensityId, intensity); }
	void SetAlpha(float alpha) { if(HasDiffuse()) { auto c = GetDiffuse(); c.SetAlpha(alpha); SetDiffuse(c); } }

	video::ColorF GetDiffuse() const
	{
		if(HasDiffuse())
			return GetColor(m_DiffuseId);
		return video::ColorF();
	}
	float GetEmissive() const {
		if(HasEmissive())
			return GetFloat(m_EmissiveId);
		return 0;
	}
	float GetSpecularHardness() const
	{
		if(HasSpecularHardness())
			return GetFloat(m_SpecularHardnessId);
		return 0;
	}
	float GetSpecularIntensity() const
	{
		if(HasSpecularIntensity())
			return GetFloat(m_SpecularIntensityId);
		return 0;
	}
	float GetAlpha() const
	{
		if(HasDiffuse())
			return GetDiffuse().GetAlpha();
		return 0;
	}

private:
	/*
	TODO:
	Add more techniques.
	Add a map for each technique to map from Params to ShaderId.
	*/
	// Shared(currently not worth it).
	Pass m_Technique;
	EMaterialReqFlag m_Requirements;

	int m_DiffuseId;
	int m_EmissiveId;
	int m_SpecularHardnessId;
	int m_SpecularIntensityId;

	// Individual 
	core::PackagePuffer m_Params;
};

} // namespace video
} // namespace lux

#endif