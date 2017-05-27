#ifndef INCLUDED_ZONE_H
#define INCLUDED_ZONE_H
#include "core/Referable.h"
#include "math/vector3.h"

namespace lux
{
namespace core
{
class Randomizer;
}
namespace scene
{

class Zone : public Referable
{
public:
	virtual bool IsInside(const math::vector3f& point) const = 0;
	virtual math::vector3f GetPointInside(const core::Randomizer& rand) const
	{
		LUX_UNUSED(rand);
		return math::vector3f::ZERO;
	}
	virtual math::vector3f GetNormal(const math::vector3f& point) const
	{
		LUX_UNUSED(point);
		return math::vector3f::ZERO;
	}

	core::Name GetReferableType() const
	{
		static const core::Name name = "zone";
		return name;
	}
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_ZONE_H