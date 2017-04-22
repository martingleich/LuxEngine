#include "BoxCollider.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"
#include "math/CollisionHelper.h"
#include "scene/zones/ZoneSphere.h"
#include "scene/zones/ZoneBox.h"
#include "scene/SceneNode.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS_NAMED(box, lux::scene::BoxCollider);
LUX_REGISTER_REFERABLE_CLASS_NAMED(bounding_box, lux::scene::BoundingBoxCollider);

namespace lux
{
namespace scene
{

EResult BoxCollider::ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	if(query->GetType() == "line")
		return ExecuteLineQuery(owner, dynamic_cast<LineQuery*>(query), dynamic_cast<LineQueryCallback*>(result));

	VolumeQuery* vquery = dynamic_cast<VolumeQuery*>(query);
	VolumeQueryCallback* vcallback = dynamic_cast<VolumeQueryCallback*>(result);

	core::Name zoneType = vquery->GetZone()->GetReferableSubType();
	if(zoneType == "sphere")
		return ExecuteSphereQuery(owner, vquery, vquery->GetZone().As<SphereZone>(), vcallback);
	else if(zoneType == "box")
		return ExecuteBoxQuery(owner, vquery, vquery->GetZone().As<BoxZone>(), vcallback);
	else
		return EResult::NotImplemented;
}

EResult BoxCollider::ExecuteLineQuery(SceneNode* owner, LineQuery* query, LineQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	math::line3df line = query->GetLine();

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
			r.position = line.start + info.distance * line.GetVector().Normal_s();
			r.sceneCollider = this;
			procceed = result->OnCollision(owner, r);
		}
	}
	break;
	default:
		return EResult::NotImplemented;
	}

	if(!procceed)
		return EResult::Aborted;

	return EResult::Succeeded;
}

EResult BoxCollider::ExecuteSphereQuery(SceneNode* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	const math::vector3f center = zone->GetCenter();
	const float radius = zone->GetRadius();

	const math::vector3f halfSize = m_HalfSize;
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
		return EResult::NotImplemented;
	}

	if(!procceed)
		return EResult::Aborted;

	return EResult::Succeeded;
}

EResult BoxCollider::ExecuteBoxQuery(SceneNode* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result)
{
	if(!owner || !query || !result)
		return EResult::Failed;

	const math::vector3f halfSizeA = zone->GetHalfSize();
	const math::Transformation transA = owner->GetAbsoluteTransform().CombineRight(m_Transform);

	const math::vector3f halfSizeB = m_HalfSize;
	const math::Transformation transB = owner->GetAbsoluteTransform().CombineRight(m_Transform);

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(math::BoxTestBox(halfSizeA, transA, halfSizeB, transB))
			procceed = result->OnObject(owner, QueryResult(this, 0));
		break;
	default:
		return EResult::NotImplemented;
	}

	if(!procceed)
		return EResult::Aborted;

	return EResult::Succeeded;
}

EResult BoundingBoxCollider::ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result)
{
	SetHalfSize(owner->GetBoundingBox().GetExtent() / 2);
	return BoxCollider::ExecuteQuery(owner, query, result);
}
}
}
