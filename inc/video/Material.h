#ifndef INCLUDED_LUX_MATERIAL_H
#define INCLUDED_LUX_MATERIAL_H
#include "video/AbstractMaterial.h"

namespace lux
{
namespace video
{

class Material : public AbstractMaterial
{
	LX_REFERABLE_MEMBERS_API(Material, LUX_API);

public:
	LUX_API Material();
	LUX_API ~Material();

	const Pass& GetPass() const
	{
		return m_Pass;
	}

	void SetPass(const Pass& pass)
	{
		lxAssert(pass.shader != nullptr);

		m_Pass = pass;
		OnUpdateShader();
	}

	void SetShader(Shader* shader)
	{
		lxAssert(shader != nullptr);

		m_Pass.shader = shader;
		OnUpdateShader();
	}
	
	StrongRef<Shader> GetShader() const
	{
		return m_Pass.shader;
	}

	void SetValue(core::StringView name, const core::VariableAccess& data)
	{
		Param(name) = data;
	}
	
	core::VariableAccess Param(core::StringView name) const
	{
		return m_ShaderValues.FromName(name, true);
	}
	
	core::VariableAccess Param(core::StringView name)
	{
		return m_ShaderValues.FromName(name, false);
	}
	
	void SetShaderValue(u32 id, const core::VariableAccess& data)
	{
		Param(id) = data;
	}
	
	core::VariableAccess Param(int id) const
	{
		return m_ShaderValues.FromID(id, true);
	}
	
	core::VariableAccess Param(int id)
	{
		return m_ShaderValues.FromID(id, false);
	}

	void SetTexture(int layer, video::BaseTexture* texture)
	{
		auto type = m_ShaderValues.GetType();
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
			m_ShaderValues.FromID(i, false) = video::TextureLayer(texture);
	}
	void SetTexture(core::StringView str, video::BaseTexture* texture)
	{
		m_ShaderValues.FromName(str, false) = video::TextureLayer(texture);
	}
	video::TextureLayer GetTexture(core::StringView str) const
	{
		return m_ShaderValues.FromName(str, true);
	}
	void SetColor(core::StringView str, const video::ColorF& color)
	{
		m_ShaderValues.FromName(str, false) = color;
	}
	void SetColor(int id, const video::ColorF& color)
	{
		m_ShaderValues.FromID(id, false) = color;
	}
	video::ColorF GetColor(core::StringView str) const
	{
		return m_ShaderValues.FromName(str, true);
	}
	
	video::ColorF GetColor(int id) const
	{
		return m_ShaderValues.FromID(id, true);
	}
	
	void SetFloat(core::StringView str, float f)
	{
		m_ShaderValues.FromName(str, false) = f;
	}
	void SetFloat(int id, float f)
	{
		m_ShaderValues.FromID(id, false) = f;
	}
	float GetFloat(core::StringView str) const
	{
		return m_ShaderValues.FromName(str, true);
	}
	float GetFloat(int id) const
	{
		return m_ShaderValues.FromID(id, true);
	}

	core::PackagePuffer& GetValues()
	{
		return m_ShaderValues;
	}
	
	const core::PackagePuffer& GetValues() const
	{
		return m_ShaderValues;
	}

	void SetDiffuse(const video::ColorF& color) { SetColor(m_DiffuseId, color); }
	void SetEmissive(float emissive) { SetFloat(m_EmissiveId, emissive); }
	void SetSpecularHardness(float hardness) { SetFloat(m_SpecularHardnessId, hardness); }
	void SetSpecularIntensity(float intensity) { SetFloat(m_SpecularIntensityId, intensity); }
	void SetAlpha(float alpha) { auto c = GetDiffuse(); c.SetAlpha(alpha); SetDiffuse(c); }

	video::ColorF GetDiffuse() const { return GetColor(m_DiffuseId); }
	float GetEmissive() const { return GetFloat(m_EmissiveId); }
	float GetSpecuularHardness() const { return GetFloat(m_SpecularHardnessId); }
	float GetSpecularIntensity() const { return GetFloat(m_SpecularIntensityId); }
	float GetAlpha() const { return GetDiffuse().GetAlpha(); }

	void SetRequirements(EMaterialReqFlag requirements)
	{
		m_Requirement = requirements;
	}

	EMaterialReqFlag GetRequirements() const
	{
		return m_Requirement;
	}

	void SendShaderSettings(const Pass& pass, void*) const
	{
		if(pass.shader) {
			for(int i = 0; i < m_ShaderValues.GetParamCount(); ++i) {
				auto param = m_ShaderValues.FromID(i, true);
				pass.shader->SetParam(i, param.Pointer());
			}
		}
	}

protected:
	void OnUpdateShader()
	{
		m_ShaderValues.SetType(&m_Pass.shader->GetParamPackage());
		m_DiffuseId = m_ShaderValues.GetType()->GetParamIdByName("diffuse");
		m_EmissiveId = m_ShaderValues.GetType()->GetParamIdByName("emissive");
		m_SpecularHardnessId = m_ShaderValues.GetType()->GetParamIdByName("specularHardness");
		m_SpecularIntensityId = m_ShaderValues.GetType()->GetParamIdByName("specularIntensity");
	}

private:
	Pass m_Pass;
	EMaterialReqFlag m_Requirement;

	core::PackagePuffer m_ShaderValues;
	int m_DiffuseId;
	int m_EmissiveId;
	int m_SpecularHardnessId;
	int m_SpecularIntensityId;
};

} // namespace video
} // namespace lux

#endif