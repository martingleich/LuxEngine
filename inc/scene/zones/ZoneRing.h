#ifndef INCLUDED_ZONE_RING_H
#define INCLUDED_ZONE_RING_H
#include "Zone.h"
#include "core/lxRandom.h"

namespace lux
{
namespace scene
{

class RingZone : public Zone
{
public:
	RingZone() :
		m_MinRadius(1.0f),
		m_MaxRadius(1.0f),
		m_HalfHeight(1.0f)
	{
	}

	RingZone(float minRadius, float maxRadius, float height) :
		m_MinRadius(minRadius),
		m_MaxRadius(maxRadius),
		m_HalfHeight(height / 2)
	{
	}

	bool IsInside(const math::Vector3F& point) const
	{
		if(m_HalfHeight <= fabsf(point.y)) {
			const float xzsq = point.x*point.x + point.z*point.z;
			if(xzsq >= m_MinRadius*m_MinRadius && xzsq <= m_MaxRadius*m_MaxRadius)
				return true;
		}

		return false;
	}

	math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		// The squareroot of the distributions ensures a uniform disribution.
		const math::Vector2F base = rand.GetVector2Circle().Normal() * std::sqrt(rand.GetFloat(m_MinRadius*m_MinRadius, m_MaxRadius*m_MaxRadius));
		
		const float y = rand.GetFloat(-m_HalfHeight, m_HalfHeight);

		return math::Vector3F(base.x, y, base.y);
	}

	math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		if(point.y > m_HalfHeight)
			return math::Vector3F::UNIT_Y;
		if(point.y < -m_HalfHeight)
			return math::Vector3F::NEGATIVE_UNIT_Y;
		float x = point.x;
		float z = point.z;
		float len = std::sqrt(x*x + z*z);
		if(math::IsZero(len))
			return math::Vector3F::ZERO;
		else if(len < m_MinRadius)
			return math::Vector3F(-x, 0.0f, -z)/len;
		else
			return math::Vector3F(x, 0.0f, z) / len;
	}

	float GetMinRadius() const
	{
		return m_MinRadius;
	}

	float GetMaxRadius() const
	{
		return m_MaxRadius;
	}

	float GetHeight() const
	{
		return m_HalfHeight * 2;
	}

	void SetMinRadius(float r)
	{
		m_MinRadius = r;
	}

	void SetMaxRadius(float r)
	{
		m_MaxRadius = r;
	}

	void SetHeight(float h)
	{
		m_HalfHeight = h / 2;
	}

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(RingZone)(*this);
	}

	LUX_API static const core::Name TypeName;

private:
	float m_MinRadius;
	float m_MaxRadius;
	float m_HalfHeight;
};

}
}
#endif // #ifndef INCLUDED_ZONE_RING_H