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
		SetHalfSize(math::vector3f(1.0f, 1.0f, 1.0f));
	}

	BoxCollider(const math::vector3f& halfSize)
	{
		SetHalfSize(halfSize);
	}

	BoxCollider(const math::vector3f& halfSize, const math::Transformation& trans) :
		m_Transform(trans)
	{
		SetHalfSize(halfSize);
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
	virtual bool ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result);
	virtual bool ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);
	virtual bool ExecuteBoxQuery(Node* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result);

	const math::aabbox3df& GetBoundingBox() const
	{
		return m_Box;
	}

	void SetHalfSize(const math::vector3f& halfSize)
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
	math::vector3f m_HalfSize;
	math::Transformation m_Transform;

	math::aabbox3df m_Box;
};

class BoundingBoxCollider : public BoxCollider
{
public:
	BoundingBoxCollider() :
		BoxCollider(math::vector3f::ZERO, math::Transformation::DEFAULT)
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