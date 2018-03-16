#ifndef INCLUDED_ZONE_CYLINDER_H
#define INCLUDED_ZONE_CYLINDER_H
#include "scene/Zone.h"

namespace lux
{
namespace scene
{

class CylinderZone : public Zone
{
	LX_REFERABLE_MEMBERS_API(CylinderZone, LUX_API);
public:
	CylinderZone() :
		m_Radius(1.0f),
		m_HalfHeight(1.0f)
	{
	}

	CylinderZone(float radius, float height) :
		m_Radius(radius),
		m_HalfHeight(height / 2)
	{
	}

	bool IsInside(const math::Vector3F& point) const
	{
		if(m_HalfHeight <= fabsf(point.y)) {
			const float xzsq = point.x*point.x + point.z*point.z;
			if(xzsq <= m_Radius*m_Radius)
				return true;
		}

		return false;
	}

	math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		const math::Vector2F base = rand.GetVector2Circle(m_Radius);
		const float y = rand.GetFloat(-m_HalfHeight, m_HalfHeight);

		return math::Vector3F(base.x, y, base.y);
	}

	math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		if(point.y >= m_HalfHeight)
			return math::Vector3F::UNIT_Y;
		if(point.y <= -m_HalfHeight)
			return math::Vector3F::NEGATIVE_UNIT_Y;
		float x = point.x;
		float z = point.z;
		float len = std::sqrt(x*x + z*z);
		if(math::IsZero(len))
			return math::Vector3F::ZERO;
		else
			return math::Vector3F(x, 0.0f, z) / len;
	}

	float GetRadius() const
	{
		return m_Radius;
	}

	float GetHeight() const
	{
		return m_HalfHeight * 2;
	}

	void SetRadius(float r)
	{
		m_Radius = r;
	}

	void SetHeight(float h)
	{
		m_HalfHeight = h / 2;
	}

private:
	float m_Radius;
	float m_HalfHeight;
};

}
}

#endif // #ifndef INCLUDED_ZONE_POINT_H
