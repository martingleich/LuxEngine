#ifndef INCLUDED_ZONE_POINT_H
#define INCLUDED_ZONE_POINT_H
#include "Zone.h"

namespace lux
{
namespace scene
{

class PointZone : public Zone
{
public:
	PointZone() {}

	PointZone(const math::vector3f& v) :
		m_Point(v)
	{
	}

	bool IsInside(const math::vector3f& point) const
	{
		return m_Point.Equal(point);
	}

	math::vector3f GetPointInside(const core::Randomizer& rand) const
	{
		LUX_UNUSED(rand);
		return m_Point;
	}

	math::vector3f GetNormal(const math::vector3f& point) const
	{
		LUX_UNUSED(point);
		return math::vector3f::ZERO;
	}

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "point";
		return name;
	}

	void SetPoint(const math::vector3f& point)
	{
		m_Point = point;
	}

	const math::vector3f& GetPoint() const
	{
		return m_Point;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(PointZone)(*this);
	}

private:
	math::vector3f m_Point;
};

}
}

#endif // #ifndef INCLUDED_ZONE_POINT_H