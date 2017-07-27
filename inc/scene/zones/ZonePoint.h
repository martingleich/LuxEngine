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

	core::Name GetReferableType() const
	{
		static const core::Name name = "pointZone";
		return name;
	}

	void SetPoint(const math::Vector3F& point)
	{
		m_Point = point;
	}

	const math::Vector3F& GetPoint() const
	{
		return m_Point;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(PointZone)(*this);
	}
	
	static const core::Name TypeName;

private:
	math::Vector3F m_Point;
};

}
}

#endif // #ifndef INCLUDED_ZONE_POINT_H