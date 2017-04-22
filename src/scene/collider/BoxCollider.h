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

	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result);
	virtual EResult ExecuteLineQuery(SceneNode* owner, LineQuery* query, LineQueryCallback* result);
	virtual EResult ExecuteSphereQuery(SceneNode* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);
	virtual EResult ExecuteBoxQuery(SceneNode* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result);

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

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "box";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return new BoxCollider(*this);
	}

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

	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result);

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "bounding_box";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return new BoundingBoxCollider(*this);
	}

};


}
}

#endif // #ifndef INCLUDED_BOX_COLLIDER_H