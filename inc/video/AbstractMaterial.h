#ifndef INCLUDED_LUX_ABSTRACT_MATERIAL_H
#define INCLUDED_LUX_ABSTRACT_MATERIAL_H
#include "core/Referable.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

enum class EMaterialReqFlag
{
	None = 0,
	Transparent = 1,
};

enum class EMaterialTechnique
{
	Default = 0,
	ShadowCaster = 1
};

class AbstractMaterialTechnique : public ShaderParamSetCallback
{
public:
	virtual const Pass& GetPass() const = 0;
};

class AbstractMaterial : public ReferenceCounted
{
public:
	virtual EMaterialReqFlag GetRequirements() const = 0;
	virtual AbstractMaterialTechnique* GetTechnique(EMaterialTechnique tech = EMaterialTechnique::Default) const = 0;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ABSTRACT_MATERIAL_H
