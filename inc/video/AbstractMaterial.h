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
	virtual const Pass& GetPass(int index) const = 0;
	virtual int GetPassCount() const = 0;

	virtual EMaterialReqFlag GetRequirements() const = 0;

	StrongRef<AbstractMaterial> Clone() const
	{
		return CloneImpl().StaticCastStrong<AbstractMaterial>();
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ABSTRACT_MATERIAL_H
