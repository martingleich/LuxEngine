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

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return new BoxCollider(*this);
	}

	static const core::Name TypeName;

protected:
	math::Vector3F m_HalfSize;
	math::Transformation m_Transform;

	math::AABBoxF m_Box;
};

class BoundingBoxCollider : public BoxCollider
{
public:
	BoundingBoxCollider() :
		BoxCollider(math::Vector3F::ZERO, math::Transformation::DEFAULT)
	{
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return new BoundingBoxCollider(*this);
	}

	static const core::Name TypeName;
};


}
}

#endif // #ifndef INCLUDED_BOX_COLLIDER_H