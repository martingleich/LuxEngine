#ifndef INCLUDED_ZONE_CYLINDER_H
#define INCLUDED_ZONE_CYLINDER_H
#include "Zone.h"

namespace lux
{
namespace scene
{

class CylinderZone : public Zone
{
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

	bool IsInside(const math::vector3f& point) const
	{
		if(m_HalfHeight <= fabsf(point.y)) {
			const float xzsq = point.x*point.x + point.z*point.z;
			if(xzsq <= m_Radius*m_Radius)
				return true;
		}

		return false;
	}

	math::vector3f GetPointInside(const core::Randomizer& rand) const
	{
		const math::vector2f base = rand.GetVector2Circle(m_Radius);
		const float y = rand.GetFloat(-m_HalfHeight, m_HalfHeight);

		return math::vector3f(base.x, y, base.y);
	}

	math::vector3f GetNormal(const math::vector3f& point) const
	{
		if(point.y >= m_HalfHeight)
			return math::vector3f::UNIT_Y;
		if(point.y <= -m_HalfHeight)
			return math::vector3f::NEGATIVE_UNIT_Y;
		float x = point.x;
		float z = point.z;
		float len = sqrt(x*x + z*z);
		if(math::IsZero(len))
			return math::vector3f::ZERO;
		else
			return math::vector3f(x, 0.0f, z) / len;
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

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "cylinder";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(CylinderZone)(*this);
	}

private:
	float m_Radius;
	float m_HalfHeight;
};

}
}

#endif // #ifndef INCLUDED_ZONE_POINT_H
