#ifndef INCLUDED_LUX_ZONE_BOX_H
#define INCLUDED_LUX_ZONE_BOX_H
#include "scene/Zone.h"
#include "math/Transformation.h"
#include "core/lxRandom.h"

namespace lux
{
namespace scene
{

class BoxZone : public Zone
{
	LX_REFERABLE_MEMBERS_API(BoxZone, LUX_API);
public:
	BoxZone() :
		m_HalfSize(0.5f, 0.5f, 0.5f)
	{
	}

	BoxZone(const math::Vector3F& halfSize, const math::Transformation& transform = math::Transformation::DEFAULT) :
		m_HalfSize(halfSize),
		m_Transformation(transform)
	{
	}

	bool IsInside(const math::Vector3F& point) const
	{
		math::Vector3F p = m_Transformation.TransformInvPoint(point);
		if(fabsf(p.x) > m_HalfSize.x || fabsf(p.y) > m_HalfSize.y || fabsf(p.z) > m_HalfSize.z)
			return false;
		else
			return true;
	}

	math::Vector3F GetPointInside(const core::Randomizer& rand) const
	{
		return m_Transformation.TransformPoint(rand.GetVector3(-m_HalfSize, m_HalfSize));
	}

	math::Vector3F GetNormal(const math::Vector3F& point) const
	{
		math::Vector3F p = m_Transformation.TransformInvPoint(point);
		p.x /= m_HalfSize.x;
		p.y /= m_HalfSize.y;
		p.z /= m_HalfSize.z;

		return GetUnitCubeVector(p);
	}

	const math::Vector3F& GetHalfSize() const
	{
		return m_HalfSize;
	}

	const math::Transformation& GetTransformation() const
	{
		return m_Transformation;
	}

	void SetHalfSize(const math::Vector3F& halfSize)
	{
		m_HalfSize = halfSize;
	}

	void SetTransformation(const math::Transformation& transform)
	{
		m_Transformation = transform;
	}

private:
	math::Vector3F m_HalfSize;
	math::Transformation m_Transformation;
};

}
}

#endif // #ifndef INCLUDED_LUX_ZONE_BOX_H