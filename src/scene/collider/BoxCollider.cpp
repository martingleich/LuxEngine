#include "BoxCollider.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"
#include "math/CollisionHelper.h"
#include "scene/zones/ZoneSphere.h"
#include "scene/zones/ZoneBox.h"
#include "scene/Node.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::BoxCollider, "lux.collider.Box");
LX_REFERABLE_MEMBERS_SRC(lux::scene::BoundingBoxCollider, "lux.collider.BBox");

namespace lux
{
namespace scene
{

bool BoxCollider::ExecuteQuery(Node* owner, Query* query, QueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	if(query->GetType() == "line")
		return ExecuteLineQuery(owner, dynamic_cast<LineQuery*>(query), dynamic_cast<LineQueryCallback*>(result));

	VolumeQuery* vquery = dynamic_cast<VolumeQuery*>(query);
	VolumeQueryCallback* vcallback = dynamic_cast<VolumeQueryCallback*>(result);

	core::Name zoneType = vquery->GetZone()->GetReferableType();
	if(zoneType == "lux.zone.Sphere")
		return ExecuteSphereQuery(owner, vquery, vquery->GetZone().As<SphereZone>(), vcallback);
	else if(zoneType == "lux.zone.Box")
		return ExecuteBoxQuery(owner, vquery, vquery->GetZone().As<BoxZone>(), vcallback);
	else
		throw core::NotImplementedException();
}

bool BoxCollider::ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	math::Line3F line = query->GetLine();

	math::Transformation fullTransform = owner->GetAbsoluteTransform().CombineRight(m_Transform);

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(math::LineTestBox(line, m_HalfSize, fullTransform))
			procceed = result->OnObject(owner, QueryResult(this, 0));
		break;
	case Query::EQueryLevel::Collision:
	{
		math::LineBoxInfo<float> info;
		if(math::LineHitBox(line, m_HalfSize, fullTransform, &info)) {
			LineQueryResult r;
			r.colliderData = 0;
			r.distance = info.distance;
			r.normal = info.normal;
			r.position = line.start + info.distance * line.GetVector().Normal();
			r.sceneCollider = this;
			procceed = result->OnCollision(owner, r);
		}
	}
	break;
	default:
		throw core::NotImplementedException();
	}

	return procceed;
}

bool BoxCollider::ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	const math::Vector3F center = zone->GetCenter();
	const float radius = zone->GetRadius();

	const math::Vector3F halfSize = m_HalfSize;
	const math::Transformation trans = owner->GetAbsoluteTransform().CombineRight(m_Transform);

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(math::SphereHitBox(center, radius, halfSize, trans))
			procceed = result->OnObject(owner, QueryResult(this, 0));
		break;
	case Query::EQueryLevel::Collision:
	{
		math::SphereBoxInfo<float> info;
		if(math::SphereHitBox(center, radius, halfSize, trans, &info)) {
			VolumeQueryResult r;
			r.colliderData = 0;
			r.seperation = -info.seperation;
			r.penetration = info.penetration;
			r.position = info.position;
			r.sceneCollider = this;
			procceed = result->OnCollision(owner, r);
		}
		break;
	}
	default:
		throw core::NotImplementedException();
	}

	return procceed;
}

bool BoxCollider::ExecuteBoxQuery(Node* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	const math::Vector3F halfSizeA = zone->GetHalfSize();
	const math::Transformation transA = zone->GetTransformation();

	const math::Vector3F halfSizeB = m_HalfSize;
	const math::Transformation transB = owner->GetAbsoluteTransform().CombineRight(m_Transform);

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(math::BoxTestBox(halfSizeA, transA, halfSizeB, transB))
			procceed = result->OnObject(owner, QueryResult(this, 0));
		break;
	default:
		throw core::NotImplementedException();
	}

	return procceed;
}

bool BoundingBoxCollider::ExecuteQuery(Node* owner, Query* query, QueryCallback* result)
{
	SetHalfSize(owner->GetBoundingBox().GetExtent() / 2);
	return BoxCollider::ExecuteQuery(owner, query, result);
}
}
}
