#ifndef INCLUDED_SCENE_COLLIDER_H
#define INCLUDED_SCENE_COLLIDER_H
#include "core/Referable.h"
#include "core/Result.h"
#include "math/aabbox3d.h"
#include "math/triangle3d.h"

namespace lux
{
namespace scene
{
class Query;
class QueryCallback;
class SceneNode;

//! A collision object
/**
Collision objects contain collision data, and perform collision queries.
*/
class Collider : public Referable
{
public:
	//! Execute a collision query.
	/**
	\param owner The collision query to which these collider belongs.
	\param query The query to perform
	\param result The callback where to send the query results, type must match the query type.
	\return The result of the query, i.e. was the query executed with success, says nothing about the collisions.
	*/
	virtual EResult ExecuteQuery(SceneNode* owner, Query* query, QueryCallback* result) = 0;

	core::Name GetReferableType() const
	{
		return ReferableType::Collider;
	}
};

//! A triangle-base collider.
/**
This collider contains triangles,
this collider will set the QueryResult::colliderData member to the id of the hit
triangle.
*/
class TriangleCollider : public Collider
{
public:
	//! Get a triangle contained in the collider, by it's id.
	virtual const math::triangle3df& GetTriangle(u32 id) const = 0;
};

}
}

#endif // #ifndef INCLUDED_SCENE_COLLIDER_H