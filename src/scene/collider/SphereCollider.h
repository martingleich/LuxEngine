#ifndef INCLUDED_SPHERE_COLLIDER_H
#define INCLUDED_SPHERE_COLLIDER_H
#include "scene/collider/Collider.h"

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

class SphereCollider : public Collider
{
public:
	SphereCollider()
	{
		SetRadius(1.0f);
	}

	SphereCollider(const math::Vector3F& center, float radius)
	{
		SetRadius(radius);
		SetCenter(center);
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
	virtual bool ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result);
	virtual bool ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);
	virtual bool ExecuteBoxQuery(Node* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result);

	virtual const math::AABBoxF& GetBoundingBox() const
	{
		return m_Box;
	}

	void SetRadius(float radius)
	{
		m_Radius = radius;
		m_Box = math::AABBoxF(
			m_Center - math::Vector3F(m_Radius, m_Radius, m_Radius),
			m_Center + math::Vector3F(m_Radius, m_Radius, m_Radius));
	}

	void SetCenter(const math::Vector3F& center)
	{
		m_Center = center;
		m_Box = math::AABBoxF(
			m_Center - math::Vector3F(m_Radius, m_Radius, m_Radius),
			m_Center + math::Vector3F(m_Radius, m_Radius, m_Radius));
	}

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(SphereCollider)(*this);
	}

	static const core::Name TypeName;

private:
	float m_Radius;
	math::Vector3F m_Center;

	math::AABBoxF m_Box;
};

class BoundingSphereCollider : public SphereCollider
{
public:
	BoundingSphereCollider() :
		SphereCollider(math::Vector3F::ZERO, 0.0f)
	{
	}

	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(BoundingSphereCollider)(*this);
	}

	static const core::Name TypeName;
};

}
}

#endif // #ifndef INCLUDED_SPHERE_COLLIDER_H