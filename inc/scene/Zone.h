#ifndef INCLUDED_LUX_ZONE_H
#define INCLUDED_LUX_ZONE_H
#include "core/Referable.h"
#include "math/Vector3.h"

namespace lux
{
namespace core
{
class Randomizer;
}
namespace scene
{

class Zone : public core::Referable
{
public:
	virtual bool IsInside(const math::Vector3F& point) const = 0;
	virtual math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		LUX_UNUSED(rand);
		return math::Vector3F::ZERO;
	}
	virtual math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		LUX_UNUSED(point);
		return math::Vector3F::ZERO;
	}
	StrongRef<Zone> Clone() const
	{
		return CloneImpl().StaticCastStrong<Zone>();
	}
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ZONE_H