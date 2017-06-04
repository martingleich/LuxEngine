#ifndef INCLUDED_MATERIAL_H
#define INCLUDED_MATERIAL_H
#include "resources/Resource.h"
#include "core/ParamPackage.h"

#include "video/Color.h"

namespace lux
{
namespace video
{
class TextureLayer;
class MaterialRenderer;

class Material : public core::Resource
{
public:
	virtual ~Material() {}

	////////////////////////////////////////////////////////////////////

	virtual void SetRenderer(MaterialRenderer* renderer) = 0;
	virtual MaterialRenderer* GetRenderer() const = 0;

	////////////////////////////////////////////////////////////////////

	virtual void CopyFrom(const Material* other) = 0;
	virtual bool Equal(const Material* other) const = 0;

	////////////////////////////////////////////////////////////////////

	virtual void SetAmbient(float ambient) = 0;
	virtual float GetAmbient() const = 0;

	virtual void SetDiffuse(const Colorf& diffuse) = 0;
	virtual const Colorf& GetDiffuse() const = 0;

	virtual void SetAlpha(float alpha) = 0;
	virtual float GetAlpha() const = 0;

	virtual void SetEmissive(const Colorf& emissive) = 0;
	virtual const Colorf& GetEmissive() const = 0;

	virtual void SetSpecular(const Colorf& specular) = 0;
	virtual const Colorf& GetSpecular() const = 0;

	virtual void SetShininess(float shininess) = 0;
	virtual float GetShininess() const = 0;

	virtual void SetPower(float power) = 0;
	virtual float GetPower() const = 0;

	////////////////////////////////////////////////////////////////////

	virtual core::PackageParam Param(const string_type& name) = 0;
	virtual core::PackageParam Param(const string_type& name) const = 0;
	virtual core::PackageParam Param(u32 id) = 0;
	virtual core::PackageParam Param(u32 id) const = 0;

	virtual core::PackageParam Layer(u32 layer) = 0;
	virtual core::PackageParam Layer(u32 layer) const = 0;

	////////////////////////////////////////////////////////////////////

	virtual u32 GetTextureCount() const = 0;
	virtual u32 GetParamCount() const = 0;

	////////////////////////////////////////////////////////////////////

	core::Name GetReferableSubType() const
	{
		return core::ResourceType::Material;
	}

	virtual StrongRef<Referable> Clone() const = 0;
};

} // namespace video
} // namespace lux

#endif