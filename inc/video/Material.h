#ifndef INCLUDED_MATERIAL_H
#define INCLUDED_MATERIAL_H
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
		GetValue(name) = data;
	}
	
	core::VariableAccess GetValue(const core::String& name) const
	{
		return m_ShaderValues.FromName(name, true);
	}
	
	core::VariableAccess GetValue(const core::String& name)
	{
		return m_ShaderValues.FromName(name, false);
	}
	
	void SetShaderValue(u32 id, const core::VariableAccess& data)
	{
		GetValue(id) = data;
	}
	
	core::VariableAccess GetValue(u32 id) const
	{
		return m_ShaderValues.FromID(id, true);
	}
	
	core::VariableAccess GetValue(u32 id)
	{
		return m_ShaderValues.FromID(id, false);
	}

	core::PackagePuffer& GetValues()
	{
		return m_ShaderValues;
	}
	
	const core::PackagePuffer& GetValues() const
	{
		return m_ShaderValues;
	}

	void SetTexture(u32 id, video::BaseTexture* texture)
	{
		m_ShaderValues.FromType(core::Types::Texture(), id, false) = texture;
	}

	void SetDiffuse(const video::ColorF& color) { m_Pass.diffuse = color; }
	void SetSpecular(const video::ColorF& color) { m_Pass.specular = color; }
	void SetEmissive(const video::ColorF& color) { m_Pass.emissive = color; }
	void SetShininess(float shininess) { m_Pass.shininess = shininess; }
	void SetAmbient(float ambient) { m_Pass.ambient = ambient; }
	void SetAlpha(float alpha) { m_Pass.diffuse.SetAlpha(alpha); }

	video::ColorF GetDiffuse() const { return m_Pass.diffuse; }
	video::ColorF GetSpecular() const { return m_Pass.specular; }
	video::ColorF GetEmissive() const { return m_Pass.emissive; }
	float GetShininess() const { return m_Pass.shininess; }
	float GetAmbient() const { return m_Pass.ambient; }
	float GetAlpha() const { return m_Pass.diffuse.GetAlpha(); }

	void SetRequirements(EMaterialRequirement requirements)
	{
		m_Requirement = requirements;
	}

	EMaterialRequirement GetRequirements() const
	{
		return m_Requirement;
	}

	void SendShaderSettings(const Pass& pass, void*) const
	{
		if(pass.shader) {
			for(u32 i = 0; i < m_ShaderValues.GetParamCount(); ++i) {
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
	EMaterialRequirement m_Requirement;

	core::PackagePuffer m_ShaderValues;
};

} // namespace video
} // namespace lux

#endif