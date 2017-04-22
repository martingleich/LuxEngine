#include "SphereCollider.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"
#include "math/CollisionHelper.h"
#include "scene/zones/ZoneSphere.h"
#include "scene/zones/ZoneBox.h"
#include "scene/SceneNode.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS_NAMED(sphere, lux::scene::SphereCollider);
LUX_REGISTER_REFERABLE_CLASS_NAMED(bounding_sphere, lux::scene::BoundingSphereCollider);

namespace lux
{
namespace scene
{

EResult SphereCollider::ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	if(query->GetType() == "line")
		return ExecuteLineQuery(owner, dynamic_cast<LineQuery*>(query), dynamic_cast<LineQueryCallback*>(result));

	VolumeQuery* vquery = dynamic_cast<VolumeQuery*>(query);

	core::Name zoneType = vquery->GetZone()->GetReferableSubType();
	if(zoneType == "sphere")
		return ExecuteSphereQuery(owner, vquery, vquery->GetZone().As<SphereZone>(), dynamic_cast<VolumeQueryCallback*>(result));
	else if(zoneType == "box")
		return ExecuteBoxQuery(owner, vquery, vquery->GetZone().As<BoxZone>(), dynamic_cast<VolumeQueryCallback*>(result));
	else
		return EResult::NotImplemented;
}

EResult SphereCollider::ExecuteLineQuery(SceneNode* owner, LineQuery* query, LineQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	math::line3df line = query->GetLine();

	math::vector3f center = owner->GetAbsoluteTransform().TransformPoint(m_Center);
	float radius = owner->GetAbsoluteTransform().scale * m_Radius;

	math::LineSphereInfo<float> info;
	bool hit = math::LineHitSphere(line, center, radius, &info);

	if(hit) {
		bool procceed = true;
		switch(query->GetLevel()) {
		case Query::EQueryLevel::Object:
			procceed = result->OnObject(owner, QueryResult(this, 0));
			break;
		case Query::EQueryLevel::Collision:
		{
			LineQueryResult data;
			data.colliderData = 0;
			data.distance = info.distance;
			data.normal = info.normal;
			data.position = line.start + info.distance * line.GetVector();
			data.sceneCollider = this;
			procceed = result->OnCollision(owner, data);
			break;
		}
		default:
			return EResult::NotImplemented;
		}

		if(!procceed)
			return EResult::Aborted;
	}

	return EResult::Succeeded;
}

EResult SphereCollider::ExecuteSphereQuery(SceneNode* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	const math::vector3f centerA = owner->GetAbsoluteTransform().TransformPoint(m_Center);
	const float radiusA = owner->GetAbsoluteTransform().scale * m_Radius;

	const math::vector3f centerB = zone->GetCenter();
	const float radiusB = zone->GetRadius();

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(math::SphereHitSphere(centerA, radiusA, centerB, radiusB))
			procceed = result->OnObject(owner, QueryResult(this, 0));
		break;
	case Query::EQueryLevel::Collision:
	{
		math::SphereSphereInfo<float> info;
		if(math::SphereHitSphere(centerA, radiusA, centerB, radiusB, &info)) {
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
		return EResult::NotImplemented;
	}

	if(!procceed)
		return EResult::Aborted;

	return EResult::Succeeded;
}

EResult SphereCollider::ExecuteBoxQuery(SceneNode* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	const math::vector3f center = owner->GetAbsoluteTransform().TransformPoint(m_Center);
	const float radius = owner->GetAbsoluteTransform().scale * m_Radius;

	const math::vector3f halfSize = zone->GetHalfSize();
	const math::Transformation trans = zone->GetTransformation();

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
			r.seperation = info.seperation;
			r.penetration = info.penetration;
			r.position = info.position;
			r.sceneCollider = this;
			procceed = result->OnCollision(owner, r);
		}
		break;
	}
	default:
		return EResult::NotImplemented;
	}

	if(!procceed)
		return EResult::Aborted;

	return EResult::Succeeded;
}

EResult BoundingSphereCollider::ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result)
{
	SetRadius(owner->GetBoundingBox().GetExtent().Average() / 2);
	return SphereCollider::ExecuteQuery(owner, query, result);
}
}
}
