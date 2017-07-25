#ifndef INCLUDED_MESH_COLLIDER_H
#define INCLUDED_MESH_COLLIDER_H
#include "scene/collider/Collider.h"
#include "math/triangle3d.h"
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
public:
	MeshCollider()
	{
	}

	MeshCollider(video::Mesh* mesh);
	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result);
	virtual bool ExecuteLineQuery(Node* owner, LineQuery* query, LineQueryCallback* result);
	virtual bool ExecuteSphereQuery(Node* owner, VolumeQuery* query, SphereZone* zone, VolumeQueryCallback* result);

	const math::aabbox3df& GetBoundingBox() const
	{
		return m_BoundingBox;
	}

	const math::triangle3df& GetTriangle(u32 id) const
	{
		return m_Triangles[id];
	}

	core::Name GetReferableType() const
	{
		return TypeName;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(MeshCollider)(*this);
	}

	static const core::Name TypeName;

private:
	struct FindEntry
	{
		size_t id;
		float distanceSq;
		math::vector3f pos;

		FindEntry() = default;
		FindEntry(size_t i, float ds, const math::vector3f& p) :
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
	bool SelectFirstTriangle(const math::line3df& line, math::vector3f& pos, size_t& triId, float& distance, bool testOnly);

private:
	core::Array<math::triangle3df> m_Triangles;
	core::Array<FindEntry> m_Temp;
	math::aabbox3df m_BoundingBox;
};

}
}

#endif // #ifndef INCLUDED_MESH_COLLIDER_H
