#ifndef INCLUDED_LUX_MESH_COLLIDER_H
#define INCLUDED_LUX_MESH_COLLIDER_H
#include "scene/Collider.h"
#include "math/Triangle3.h"
#include "core/lxArray.h"

namespace lux
{
namespace  video
{
class Mesh;
}
namespace scene
{
class LineQuery;
class VolumeQuery;
class SphereZone;
class BoxZone;
class LineQueryCallback;
class VolumeQueryCallback;

class MeshCollider : public TriangleCollider
{
	LX_REFERABLE_MEMBERS_API(MeshCollider, LUX_API);
public:
	MeshCollider()
	{
	}

	MeshCollider(video::Mesh* mesh);
	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
	virtual bool ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result);
	virtual bool ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);

	const math::AABBoxF& GetBoundingBox() const
	{
		return m_BoundingBox;
	}

	const math::Triangle3F& GetTriangle(int id) const
	{
		return m_Triangles[id];
	}

private:
	struct FindEntry
	{
		int id;
		float distanceSq;
		math::Vector3F pos;

		FindEntry() = default;
		FindEntry(int i, float ds, const math::Vector3F& p) :
			id(i),
			distanceSq(ds),
			pos(p)
		{
		}

		bool operator<(const FindEntry& other) const
		{
			return distanceSq < other.distanceSq;
		}
	};

private:
	bool SelectFirstTriangle(const math::Line3F& line, math::Vector3F& pos, int& triId, float& distance, bool testOnly);

private:
	core::Array<math::Triangle3F> m_Triangles;
	core::Array<FindEntry> m_Temp;
	math::AABBoxF m_BoundingBox;
};

}
}

#endif // #ifndef INCLUDED_LUX_MESH_COLLIDER_H
