#include "MeshCollider.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"
#include "math/CollisionHelper.h"
#include "scene/zones/ZoneSphere.h"
#include "scene/zones/ZoneBox.h"
#include "scene/Node.h"
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/VertexFormats.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "core/ReferableRegister.h"

const lux::core::Name lux::scene::MeshCollider::TypeName("lux.collider.Mesh");

LUX_REGISTER_REFERABLE_CLASS(lux::scene::MeshCollider::TypeName, lux::scene::MeshCollider);

namespace lux
{
namespace scene
{

MeshCollider::MeshCollider(video::Mesh* mesh)
{
	m_BoundingBox = mesh->GetBoundingBox();
	size_t faceCount = mesh->GetGeometry()->GetPrimitiveCount();

	m_Triangles.Clear();
	m_Triangles.Reserve(faceCount);

	auto sub = mesh->GetGeometry();
	auto indices = sub->GetIndices();
	auto vertices = sub->GetVertices();
	u32 offset = sub->GetVertexFormat().GetElement(video::VertexElement::EUsage::Position).offset;
	u32 stride = sub->GetVertexFormat().GetStride();
	const u8* data = (const u8*)vertices->Pointer_c(0, vertices->GetSize());
	for(u32 j = 0; j < indices->GetSize(); j += 3) {
		u32 i0 = indices->GetIndex(j + 0);
		u32 i1 = indices->GetIndex(j + 1);
		u32 i2 = indices->GetIndex(j + 2);
		math::Triangle3F tri(
			*(const math::Vector3F*)(data + stride*i0 + offset),
			*(const math::Vector3F*)(data + stride*i1 + offset),
			*(const math::Vector3F*)(data + stride*i2 + offset));

		m_Triangles.PushBack(tri);
	}
}

bool MeshCollider::ExecuteQuery(Node* owner, Query* query, QueryCallback* result)
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
	else
		throw core::NotImplementedException();
}

bool MeshCollider::ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	math::Line3F line = query->GetLine();
	const auto& trans = owner->GetAbsoluteTransform();
	math::Line3F transLine = math::Line3F(
		trans.TransformInvPoint(line.start),
		trans.TransformInvPoint(line.end));

	math::Vector3F pos;
	size_t id;
	float distance;
	bool found = SelectFirstTriangle(transLine, pos, id, distance, query->GetLevel() == Query::EQueryLevel::Object);

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		if(found)
			procceed = result->OnObject(owner, QueryResult(this, id));
		break;
	case Query::EQueryLevel::Collision:
	{
		if(found) {
			LineQueryResult r;
			r.colliderData = id;
			r.distance = distance;
			r.normal = trans.TransformDir(m_Triangles[id].GetNormal());
			r.position = trans.TransformPoint(pos);
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

bool MeshCollider::ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result)
{
	LX_CHECK_NULL_ARG(owner);
	LX_CHECK_NULL_ARG(query);
	LX_CHECK_NULL_ARG(result);

	auto& absTrans = owner->GetAbsoluteTransform();

	auto center = absTrans.TransformInvPoint(zone->GetCenter());
	auto radius = zone->GetRadius() / absTrans.scale;

	bool procceed = true;
	switch(query->GetLevel()) {
	case Query::EQueryLevel::Object:
		for(auto it = m_Triangles.First(); it != m_Triangles.End(); ++it) {
			bool hit = math::TriangleTestSphere(
				center, radius,
				*it);
			if(hit) {
				QueryResult r(this, core::IteratorDistance(m_Triangles.First(), it));
				procceed = result->OnObject(owner, r);
				break;
			}
		}
		break;
	default:
		throw core::NotImplementedException();
	}

	return procceed;
}

bool MeshCollider::SelectFirstTriangle(const math::Line3F& line, math::Vector3F& out_pos, size_t& triId, float& distance, bool testOnly)
{
	if(!m_BoundingBox.IntersectWithLine(line))
		return false;

	size_t i = 0;
	for(auto it = m_Triangles.First(); it != m_Triangles.End(); ++it) {
		math::Vector3F pos;
		if(it->IntersectWithLineBary(line, &pos)) {
			if(testOnly)
				return true;
			m_Temp.PushBack(FindEntry(
				i,
				line.start.GetDistanceToSq(pos),
				pos));
		}
		++i;
	}

	if(!m_Temp.IsEmpty()) {
		m_Temp.Sort();
		out_pos = m_Temp[0].pos;
		triId = m_Temp[0].id;
		distance = std::sqrt(m_Temp[0].distanceSq);
		m_Temp.Clear();
		return true;
	}

	return false;
}

}
}
