#ifndef INCLUDED_LUX_SCENE_COLLIDER_H
#define INCLUDED_LUX_SCENE_COLLIDER_H
#include "core/Referable.h"
#include "math/AABBox.h"
#include "math/Triangle3.h"

namespace lux
{
namespace scene
{
class Query;
class QueryCallback;
class Node;

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
	\return True if the run trough, false if it was aborted
	*/
	virtual bool ExecuteQuery(Node* owner, Query* query, QueryCallback* result) = 0;

	StrongRef<Collider> Clone() const
	{
		return CloneImpl().StaticCastStrong<Collider>();
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
	virtual const math::Triangle3F& GetTriangle(int id) const = 0;

	StrongRef<TriangleCollider> Clone() const
	{
		return CloneImpl().StaticCastStrong<TriangleCollider>();
	}
};

}
}

#endif // #ifndef INCLUDED_LUX_SCENE_COLLIDER_H