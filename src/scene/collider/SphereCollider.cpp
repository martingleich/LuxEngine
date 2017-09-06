#include "SphereCollider.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"
#include "math/CollisionHelper.h"
#include "scene/zones/ZoneSphere.h"
#include "scene/zones/ZoneBox.h"
#include "scene/Node.h"

#include "core/ReferableRegister.h"

const lux::core::Name lux::scene::SphereCollider::TypeName = "lux.collider.Sphere";
const lux::core::Name lux::scene::BoundingSphereCollider::TypeName = "lux.collider.BSphere";

LUX_REGISTER_REFERABLE_CLASS_NAMED(sphere, lux::scene::SphereCollider::TypeName, lux::scene::SphereCollider);
LUX_REGISTER_REFERABLE_CLASS_NAMED(bounding_sphere, lux::scene::BoundingSphereCollider::TypeName, lux::scene::BoundingSphereCollider);

namespace lux
{
namespace scene
{

bool SphereCollider::ExecuteQuery(Node* owner, Query* query, QueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	if(query->GetType() == "line")
		return ExecuteLineQuery(owner, dynamic_cast<LineQuery*>(query), dynamic_cast<LineQueryCallback*>(result));

	VolumeQuery* vquery = dynamic_cast<VolumeQuery*>(query);

	core::Name zoneType = vquery->GetZone()->GetReferableType();
	if(zoneType == SphereZone::TypeName)
		return ExecuteSphereQuery(owner, vquery, vquery->GetZone().As<SphereZone>(), dynamic_cast<VolumeQueryCallback*>(result));
	else if(zoneType == BoxZone::TypeName)
		return ExecuteBoxQuery(owner, vquery, vquery->GetZone().As<BoxZone>(), dynamic_cast<VolumeQueryCallback*>(result));
	else
		throw core::NotImplementedException();
}

bool SphereCollider::ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result)
{
	math::Line3F line = query->GetLine();

	math::Vector3F center = owner->GetAbsoluteTransform().TransformPoint(m_Center);
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
			throw core::NotImplementedException();
		}

		if(!procceed)
			return false;
	}

	return true;
}

bool SphereCollider::ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result)
{
	LX_CHECK_NULL_ARG(zone);

	const math::Vector3F centerA = owner->GetAbsoluteTransform().TransformPoint(m_Center);
	const float radiusA = owner->GetAbsoluteTransform().scale * m_Radius;

	const math::Vector3F centerB = zone->GetCenter();
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
			r.seperation = info.seperation;
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

	if(!procceed)
		return false;

	return true;
}

bool SphereCollider::ExecuteBoxQuery(Node* owner, VolumeQuery* query, BoxZone* zone, VolumeQueryCallback* result)
{
	LX_CHECK_NULL_ARG(zone);

	const math::Vector3F center = owner->GetAbsoluteTransform().TransformPoint(m_Center);
	const float radius = owner->GetAbsoluteTransform().scale * m_Radius;

	const math::Vector3F halfSize = zone->GetHalfSize();
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
		throw core::NotImplementedException();
	}

	return procceed;
}

bool BoundingSphereCollider::ExecuteQuery(Node* owner, Query* query, QueryCallback* result)
{
	SetRadius(owner->GetBoundingBox().GetExtent().Average() / 2);
	return SphereCollider::ExecuteQuery(owner, query, result);
}
}
}
