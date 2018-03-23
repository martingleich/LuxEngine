#ifndef INCLUDED_LUX_ZONE_POINT_H
#define INCLUDED_LUX_ZONE_POINT_H
#include "scene/Zone.h"

namespace lux
{
namespace scene
{

class PointZone : public Zone
{
	LX_REFERABLE_MEMBERS_API(PointZone, LUX_API);
public:
	PointZone() {}

	PointZone(const math::Vector3F& v) :
		m_Point(v)
	{
	}

	bool IsInside(const math::Vector3F& point) const
	{
		return math::IsEqual(point, m_Point);
	}

	math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		LUX_UNUSED(rand);
		return m_Point;
	}

	math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		LUX_UNUSED(point);
		return math::Vector3F::ZERO;
	}

	void SetPoint(const math::Vector3F& point)
	{
		m_Point = point;
	}

	const math::Vector3F& GetPoint() const
	{
		return m_Point;
	}

private:
	math::Vector3F m_Point;
};

}
}

#endif // #ifndef INCLUDED_LUX_ZONE_POINT_H