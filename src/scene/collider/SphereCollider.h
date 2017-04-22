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

	SphereCollider(const math::vector3f& center, float radius)
	{
		SetRadius(radius);
		SetCenter(center);
	}

	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result);
	virtual EResult ExecuteLineQuery(SceneNode* owner, LineQuery* query, LineQueryCallback* result);
	virtual EResult ExecuteSphereQuery(SceneNode* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);
	virtual EResult ExecuteBoxQuery(SceneNode* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result);

	virtual const math::aabbox3df& GetBoundingBox() const
	{
		return m_Box;
	}

	void SetRadius(float radius)
	{
		m_Radius = radius;
		m_Box = math::aabbox3df(
			m_Center - math::vector3f(m_Radius, m_Radius, m_Radius),
			m_Center + math::vector3f(m_Radius, m_Radius, m_Radius));
	}

	void SetCenter(const math::vector3f& center)
	{
		m_Center = center;
		m_Box = math::aabbox3df(
			m_Center - math::vector3f(m_Radius, m_Radius, m_Radius),
			m_Center + math::vector3f(m_Radius, m_Radius, m_Radius));
	}

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "sphere";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return new SphereCollider(*this);
	}

private:
	float m_Radius;
	math::vector3f m_Center;

	math::aabbox3df m_Box;
};

class BoundingSphereCollider : public SphereCollider
{
public:
	BoundingSphereCollider() :
		SphereCollider(math::vector3f::ZERO, 0.0f)
	{
	}

	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result);

	core::Name GetReferableSubType() const
	{
		static const core::Name name = "bounding_sphere";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return new BoundingSphereCollider(*this);
	}
};

}
}

#endif // #ifndef INCLUDED_SPHERE_COLLIDER_H