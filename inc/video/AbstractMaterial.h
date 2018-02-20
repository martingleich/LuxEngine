#ifndef INCLUDED_LUX_ABSTRACT_MATERIAL_H
#define INCLUDED_LUX_ABSTRACT_MATERIAL_H
#include "core/Referable.h"
#include "video/Pass.h"

namespace lux
{
namespace video
{

class AbstractMaterial : public ShaderParamSetCallback, public Referable
{
public:
	virtual const Pass& GetPass(u32 index) const = 0;
	virtual u32 GetPassCount() const = 0;

	virtual EMaterialRequirement GetRequirements() const = 0;

	StrongRef<AbstractMaterial> Clone() const
	{
		return CloneImpl().StaticCastStrong<AbstractMaterial>();
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ABSTRACT_MATERIAL_H