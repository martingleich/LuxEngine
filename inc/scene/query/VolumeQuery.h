#ifndef INCLUDED_VOLUME_QUERY_H
#define INCLUDED_VOLUME_QUERY_H
#include "scene/query/Query.h"
#include "scene/query/QueryCallback.h"
#include "scene/zones/Zone.h"

namespace lux
{
namespace scene
{

//! A volume collision query.
/**
Performs a collision query with a given Zone.
WARNING: Not all zones are supported, watch out for NotImplemented results.
*/
class VolumeQuery : public Query
{
public:
	VolumeQuery() :
		m_Zone(nullptr)
	{}

	VolumeQuery(Node* rootNode, Zone* zone = nullptr) :
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
	math::Vector3F position;
	//! The direction in which to move the collider object, to resolve the collision, in world-coordinates.
	math::Vector3F seperation;
	//! The depth of the collision, if the collider is moved by penetration*seperation, the collision will be resolved.
	float penetration;
};

//! A volume query result callback
class VolumeQueryCallback : public QueryCallback
{
public:
	//! Called for each "real" collision.
	/**
	Will be called for each object in a EQueryLevel::Collision query.
	Defaults to all call to the OnObject method.
	*/
	virtual bool OnCollision(Node* node, const VolumeQueryResult& result)
	{
		LUX_UNUSED(result);
		return QueryCallback::OnObject(node, result);
	}
};

}
}

#endif // #ifndef INCLUDED_VOLUME_QUERY_H