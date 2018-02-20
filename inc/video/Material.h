#ifndef INCLUDED_MATERIAL_H
#define INCLUDED_MATERIAL_H
#include "video/AbstractMaterial.h"

namespace lux
{
namespace video
{

#pragma warning(disable: 4100)
class Material : public AbstractMaterial
{
	LX_REFERABLE_MEMBERS_API(Material, LUX_API);

public:
	u32 GetPassCount() const
	{
		return 1;
	}
	const Pass& GetPass() const
	{
		return GetPass(0);
	}

	Pass& GetPass()
	{
		return m_Pass;
	}

	void SendShaderSettings(u32 passId, const Pass& pass, void* userParam) const
	{
	}

	void SetValue(const core::String& name)
	{
	}

	void SetTexture(u32 id, video::BaseTexture* texture)
	{
		m_Pass.layers.Resize(id + 1);
		m_Pass.layers[id].texture = texture;
	}

	void SetDiffuse(const video::Colorf& color)
	{
		m_Pass.diffuse = color;
	}

	void SetSpecular(const video::Colorf& color)
	{
		m_Pass.specular = color;
	}

	void SetEmissive(const video::Colorf& color)
	{
		m_Pass.emissive = color;
	}

	void SetShininess(float shininess)
	{
		m_Pass.shininess = shininess;
	}

	void SetAlpha(float alpha)
	{
		m_Pass.diffuse.SetAlpha(alpha);
	}

	video::Colorf GetDiffuse() const { return m_Pass.diffuse; }
	video::Colorf GetSpecular() const { return m_Pass.specular; }
	video::Colorf GetEmissive() const { return m_Pass.emissive; }

	void CopyFrom(const AbstractMaterial* other)
	{
		m_Pass = other->GetPass(0);
		m_Requirement = other->GetRequirements();
	}

	void SetRequirements(EMaterialRequirement requirements)
	{
		m_Requirement = requirements;
	}

	EMaterialRequirement GetRequirements() const
	{
		return m_Requirement;
	}

protected:
	const Pass& GetPass(u32) const
	{
		return m_Pass;
	}
private:
	Pass m_Pass;
	EMaterialRequirement m_Requirement;
};

#pragma warning(default: 4100)

} // namespace video
} // namespace lux

#endif