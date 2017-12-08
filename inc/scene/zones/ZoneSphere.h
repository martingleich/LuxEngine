#ifndef INCLUDED_ZONE_SPHERE_H
#define INCLUDED_ZONE_SPHERE_H
#include "Zone.h"
#include "core/lxRandom.h"

namespace lux
{
namespace scene
{

class SphereZone : public Zone
{
public:
	LX_REFERABLE_MEMBERS_API(LUX_API);

	SphereZone() :
		m_Radius(1.0f)
	{
	}

	SphereZone(float radius, const math::Vector3F& center = math::Vector3F::ZERO) :
		m_Center(center),
		m_Radius(radius)
	{
	}

	bool IsInside(const math::Vector3F& point) const
	{
		return point.GetDistanceToSq(m_Center) <= m_Radius*m_Radius;
	}

	math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		return rand.GetVector3Sphere(m_Radius) + m_Center;
	}

	math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		return (point - m_Center).Normal();
	}

	const math::Vector3F& GetCenter() const
	{
		return m_Center;
	}

	float GetRadius() const
	{
		return m_Radius;
	}

	void SetCenter(const math::Vector3F& center)
	{
		m_Center = center;
	}

	void SetRadius(float radius)
	{
		m_Radius = radius;
	}

private:
	math::Vector3F m_Center;
	float m_Radius;
};

}
}

#endif // #ifndef INCLUDED_ZONE_SPHERE_H