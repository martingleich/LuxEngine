#ifndef INCLUDED_ZONE_H
#define INCLUDED_ZONE_H
#include "core/Referable.h"
#include "core/lxRandom.h"

namespace lux
{
namespace scene
{

class Zone : public Referable
{
public:
	virtual bool IsInside(const math::vector3f& point) const = 0;
	virtual math::vector3f GetPointInside(const core::Randomizer& rand) const
	{
		return math::vector3f::ZERO;
	}
	virtual math::vector3f GetNormal(const core::Randomizer& rand, const math::vector3f& point) const
	{
		return rand.GetVector3Sphere();
	}

	core::Name GetReferableType() const
	{
		static const core::Name name = "zone";
		return name;
	}

};

}
}

#endif // #ifndef INCLUDED_ZONE_H