#ifndef INCLUDED_BOX_COLLIDER_H
#define INCLUDED_BOX_COLLIDER_H
#include "scene/collider/Collider.h"
#include "math/Transformation.h"

namespace lux
{
namespace scene
{

class LineQuery;
class VolumeQuery;
class SphereZone;
class BoxZone;
class LineQueryCallback;
class VolumeQueryCallback;

class BoxCollider : public Collider
{
	LX_REFERABLE_MEMBERS_API(BoxCollider, LUX_API);
public:
	BoxCollider()
	{
		SetHalfSize(math::Vector3F(1.0f, 1.0f, 1.0f));
	}

	BoxCollider(const math::Vector3F& halfSize)
	{
		SetHalfSize(halfSize);
	}

	BoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) :
		m_Transform(trans)
	{
		SetHalfSize(halfSize);
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
	virtual bool ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result);
	virtual bool ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);
	virtual bool ExecuteBoxQuery(Node* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result);

	const math::AABBoxF& GetBoundingBox() const
	{
		return m_Box;
	}

	void SetHalfSize(const math::Vector3F& halfSize)
	{
		m_HalfSize = halfSize;
		m_Box.Set(m_Transform.TransformPoint(m_HalfSize));
		m_Box.AddPoint(m_Transform.TransformPoint(-m_HalfSize));
	}

protected:
	math::Vector3F m_HalfSize;
	math::Transformation m_Transform;

	math::AABBoxF m_Box;
};

class BoundingBoxCollider : public BoxCollider
{
	LX_REFERABLE_MEMBERS_API(BoundingBoxCollider, LUX_API);
public:
	BoundingBoxCollider() :
		BoxCollider(math::Vector3F::ZERO, math::Transformation::DEFAULT)
	{
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
};


}
}

#endif // #ifndef INCLUDED_BOX_COLLIDER_H