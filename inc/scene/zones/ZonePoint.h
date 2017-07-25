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
		return math::IsEqual(point, m_Point);
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

	core::Name GetReferableType() const
	{
		static const core::Name name = "pointZone";
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
	
	static const core::Name TypeName;

private:
	math::vector3f m_Point;
};

}
}

#endif // #ifndef INCLUDED_ZONE_POINT_H