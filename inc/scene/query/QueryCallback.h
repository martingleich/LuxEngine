#ifndef INCLUDED_QUERY_CALLBACK_H
#define INCLUDED_QUERY_CALLBACK_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace scene
{
class SceneNode;
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

class QueryCallback : public ReferenceCounted
{
public:
	virtual bool OnObject(SceneNode* node, const QueryResult& result)
	{
		LUX_UNUSED(node);
		LUX_UNUSED(result);
		return true;
	}
};

}
}

#endif // #ifndef INCLUDED_QUERY_CALLBACK_H
