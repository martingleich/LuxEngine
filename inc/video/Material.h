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

	u32 GetPassCount() const
	{
		return 1;
	}

	const Pass& GetPass() const
	{
		return GetPass(0);
	}

	void SetPass(const Pass& pass)
	{
		lxAssert(pass.shader != nullptr);

		m_Pass = pass;

		if(m_ShaderValues.GetType() != &m_Pass.shader->GetParamPackage())
			m_ShaderValues.SetType(&m_Pass.shader->GetParamPackage());
	}

	void SetShader(Shader* shader)
	{
		lxAssert(shader != nullptr);

		m_Pass.shader = shader;
		if(m_ShaderValues.GetType() != &m_Pass.shader->GetParamPackage())
			m_ShaderValues.SetType(&m_Pass.shader->GetParamPackage());
	}
	
	StrongRef<Shader> GetShader() const
	{
		return m_Pass.shader;
	}

	void SetValue(const core::String& name, const core::VariableAccess& data)
	{
		Param(name) = data;
	}
	
	core::VariableAccess Param(const core::String& name) const
	{
		return m_ShaderValues.FromName(name, true);
	}
	
	core::VariableAccess Param(const core::String& name)
	{
		return m_ShaderValues.FromName(name, false);
	}
	
	void SetShaderValue(u32 id, const core::VariableAccess& data)
	{
		Param(id) = data;
	}
	
	core::VariableAccess Param(u32 id) const
	{
		return m_ShaderValues.FromID(id, true);
	}
	
	core::VariableAccess Param(u32 id)
	{
		return m_ShaderValues.FromID(id, false);
	}

	void SetTexture(u32 layer, video::BaseTexture* texture)
	{
		m_ShaderValues.FromType(core::Types::Texture(), layer, false) = video::TextureLayer(texture);
	}
	void SetTexture(core::StringView str, video::BaseTexture* texture)
	{
		m_ShaderValues.FromName(str, false) = video::TextureLayer(texture);
	}
	void SetColor(core::StringView str, const video::ColorF& color)
	{
		m_ShaderValues.FromName(str, false) = color;
	}
	video::ColorF GetColor(core::StringView str) const
	{
		return m_ShaderValues.FromName(str, true);
	}
	void SetFloat(core::StringView str, float f)
	{
		m_ShaderValues.FromName(str, false) = f;
	}
	float GetFloat(core::StringView str) const
	{
		return m_ShaderValues.FromName(str, true);
	}

	core::PackagePuffer& GetValues()
	{
		return m_ShaderValues;
	}
	
	const core::PackagePuffer& GetValues() const
	{
		return m_ShaderValues;
	}

	void SetDiffuse(const video::ColorF& color) { m_Pass.diffuse = color; }
	void SetEmissive(float emissive) { m_Pass.emissive = emissive; }
	void SetShininess(float shininess) { m_Pass.shininess = shininess; }
	void SetSpecularIntensity(float specular) { m_Pass.specularIntensity = specular; }
	void SetAlpha(float alpha) { m_Pass.diffuse.SetAlpha(alpha); }

	video::ColorF GetDiffuse() const { return m_Pass.diffuse; }
	float GetEmissive() const { return m_Pass.emissive; }
	float GetShininess() const { return m_Pass.shininess; }
	float GetSpecularIntesity() const { return m_Pass.specularIntensity; }
	float GetAlpha() const { return m_Pass.diffuse.GetAlpha(); }

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
	const Pass& GetPass(u32) const
	{
		return m_Pass;
	}

private:
	Pass m_Pass;
	EMaterialReqFlag m_Requirement;

	core::PackagePuffer m_ShaderValues;
};

} // namespace video
} // namespace lux

#endif