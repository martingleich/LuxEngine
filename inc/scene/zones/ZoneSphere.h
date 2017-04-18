#ifndef INCLUDED_ZONE_SPHERE_H
#define INCLUDED_ZONE_SPHERE_H
#include "Zone.h"

namespace lux
{
namespace scene
{

class SphereZone : public Zone
{
public:
	SphereZone() :
		m_Radius(1.0f)
	{
	}

	SphereZone(float radius, const math::vector3f& center = math::vector3f::ZERO) :
		m_Center(center),
		m_Radius(radius)
	{
	}

	bool IsInside(const math::vector3f& point) const
	{
		return point.GetDistanceToSq(m_Center) <= m_Radius*m_Radius;
	}

	math::vector3f GetPointInside(const core::Randomizer& rand) const
	{
		return rand.GetVector3Sphere(m_Radius) + m_Center;
	}

	math::vector3f GetNormal(const core::Randomizer& rand, const math::vector3f& point) const
	{
		return (point - m_Center).Normal_s();
	}

	const math::vector3f& GetCenter() const
	{
		return m_Center;
	}

	float GetRadius() const
	{
		return m_Radius;
	}

	void SetCenter(const math::vector3f& center)
	{
		m_Center = center;
	}

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "sphere";
		return name;
	}

	void SetRadius(float radius)
	{
		m_Radius = radius;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(SphereZone)(*this);
	}

private:
	math::vector3f m_Center;
	float m_Radius;
};

}
}

#endif // #ifndef INCLUDED_ZONE_SPHERE_H