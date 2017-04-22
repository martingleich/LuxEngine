#ifndef INCLUDED_VOLUME_QUERY_H
#define INCLUDED_VOLUME_QUERY_H
#include "scene/query/Query.h"
#include "scene/zones/Zone.h"

namespace lux
{
namespace scene
{

class VolumeQuery : public Query
{
public:
	VolumeQuery(SceneNode* rootNode, Zone* zone = nullptr) :
		Query(rootNode),
		m_Zone(zone)
	{
	}

	void SetZone(Zone* zone)
	{
		m_Zone = zone;
	}

	StrongRef<Zone> GetZone() const
	{
		return m_Zone;
	}

	core::Name GetType() const
	{
		static const core::Name n = "volume";
		return n;
	}

private:
	StrongRef<Zone> m_Zone;
};

//! A single result of a volume query.
struct VolumeQueryResult : QueryResult
{
	//! The position of the collision, the position of the "worst", the deepest collision, in world-coordinates.
	math::vector3f position;
	//! The direction in which to move the collider object, to resolve the collision, in world-coordinates.
	math::vector3f seperation;
	//! The depth of the collision, if the collider is moved by penetration*seperation, the collision will be resolved.
	float penetration;
};

class VolumeQueryCallback : public QueryCallback
{
public:
	virtual bool OnCollision(SceneNode* node, const VolumeQueryResult& result)
	{
		LUX_UNUSED(result);
		return QueryCallback::OnObject(node);
	}
};

}
}

#endif // #ifndef INCLUDED_VOLUME_QUERY_H