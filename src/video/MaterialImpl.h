#ifndef INCLUDED_MATERIAL_IMPL_H
#define INCLUDED_MATERIAL_IMPL_H
#include "video/Material.h"

namespace lux
{
namespace video
{
class MaterialRenderer;

class MaterialImpl : public Material
{
public:
	MaterialImpl(MaterialRenderer* renderer);

	////////////////////////////////////////////////////////////////////

	void SetRenderer(MaterialRenderer* renderer);
	MaterialRenderer* GetRenderer() const;

	////////////////////////////////////////////////////////////////////

	void CopyFrom(const Material* _other);
	bool Equal(const Material* _other) const;

	////////////////////////////////////////////////////////////////////

	void SetAmbient(float ambient);
	float GetAmbient() const;
	void SetDiffuse(const Colorf& diffuse);
	const Colorf& GetDiffuse() const;
	void SetAlpha(float alpha);
	float GetAlpha() const;
	void SetEmissive(const Colorf& emissive);
	const Colorf& GetEmissive() const;
	void SetSpecular(const Colorf& specular);
	const Colorf& GetSpecular() const;
	void SetShininess(float shininess);
	float GetShininess() const;

	////////////////////////////////////////////////////////////////////

	core::PackageParam Param(const string_type& name);
	core::PackageParam Param(const string_type& name) const;
	core::PackageParam Param(u32 id);
	core::PackageParam Param(u32 id) const;
	core::PackageParam Layer(u32 layer);
	core::PackageParam Layer(u32 layer) const;

	////////////////////////////////////////////////////////////////////

	u32 GetTextureCount() const;
	u32 GetParamCount() const;

	////////////////////////////////////////////////////////////////////

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	float m_Ambient;
	Colorf m_Diffuse;
	Colorf m_Emissive;
	Colorf m_Specular;
	float m_Shininess;

	WeakRef<MaterialRenderer> m_Renderer;
	core::PackagePuffer m_Puffer;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_MATERIAL_IMPL_H