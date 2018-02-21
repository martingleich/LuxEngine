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
		m_Pass = pass;

		if(m_Pass.shader) {
			if(m_ShaderValues.GetType() != &m_Pass.shader->GetParamPackage())
				m_ShaderValues.SetType(&m_Pass.shader->GetParamPackage());
		}
	}
	
	void SetShader(Shader* shader)
	{
		m_Pass.shader = shader;
		if(m_Pass.shader) {
			if(m_ShaderValues.GetType() != &m_Pass.shader->GetParamPackage())
				m_ShaderValues.SetType(&m_Pass.shader->GetParamPackage());
		}
	}
	StrongRef<Shader> GetShader() const
	{
		return m_Pass.shader;
	}

	void SetShaderValue(const core::String& name, const core::VariableAccess& data)
	{
		GetShaderValue(name) = data;
	}
	core::VariableAccess GetShaderValue(const core::String& name) const
	{
		return m_ShaderValues.FromName(name, true);
	}
	core::VariableAccess GetShaderValue(const core::String& name)
	{
		return m_ShaderValues.FromName(name, false);
	}
	void SetShaderValue(u32 id, const core::VariableAccess& data)
	{
		GetShaderValue(id) = data;
	}
	core::VariableAccess GetShaderValue(u32 id) const
	{
		return m_ShaderValues.FromID(id, true);
	}
	core::VariableAccess GetShaderValue(u32 id)
	{
		return m_ShaderValues.FromID(id, false);
	}

	core::PackagePuffer& GetShaderValues()
	{
		return m_ShaderValues;
	}
	const core::PackagePuffer& GetShaderValues() const
	{
		return m_ShaderValues;
	}

	void SetTexture(u32 id, video::BaseTexture* texture)
	{
		if(m_Pass.shader) {
			m_ShaderValues.FromType(core::Types::Texture(), id, false) = texture;
		} else {
			if(id >= m_Pass.layers.Size())
				m_Pass.layers.Resize(id + 1);
			m_Pass.layers[id].texture = texture;
		}
	}

	void SetDiffuse(const video::Colorf& color) { m_Pass.diffuse = color; }
	void SetSpecular(const video::Colorf& color) { m_Pass.specular = color; }
	void SetEmissive(const video::Colorf& color) { m_Pass.emissive = color; }
	void SetShininess(float shininess) { m_Pass.shininess = shininess; }
	void SetAmbient(float ambient) { m_Pass.ambient = ambient; }
	void SetAlpha(float alpha) { m_Pass.diffuse.SetAlpha(alpha); }

	video::Colorf GetDiffuse() const { return m_Pass.diffuse; }
	video::Colorf GetSpecular() const { return m_Pass.specular; }
	video::Colorf GetEmissive() const { return m_Pass.emissive; }
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

			pass.shader->LoadSceneParams(pass);
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