#ifndef INCLUDED_QUERY_CALLBACK_H
#define INCLUDED_QUERY_CALLBACK_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace scene
{
class Node;
class Collider;

//! The base result of a query.
struct QueryResult
{
	//! The collider which trigged the query.
	Collider* sceneCollider;
	//! Additional query data, interpretation depends on the type of the collider.
	s32 colliderData;

	QueryResult() :
		sceneCollider(nullptr),
		colliderData(0)
	{}

	QueryResult(Collider* c, s32 d) :
		sceneCollider(c),
		colliderData(d)
	{}
};

//! The query result callback interface.
class QueryCallback : public ReferenceCounted
{
public:
	//! This method is called for each colliding object in the query.
	/**
	Will be called for all result objects in the query.
	\param node The scene node of the collided object.
	\param result More information about the collision.
	\return Return true to resume the query, return false to abort it.
	*/
	virtual bool OnObject(Node* node, const QueryResult& result)
	{
		LUX_UNUSED(node);
		LUX_UNUSED(result);
		return true;
	}
};

}
}

#endif // #ifndef INCLUDED_QUERY_CALLBACK_H
