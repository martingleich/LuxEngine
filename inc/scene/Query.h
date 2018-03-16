#ifndef INCLUDED_QUERY_H
#define INCLUDED_QUERY_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"

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
class QueryCallback
{
public:
	virtual ~ QueryCallback() {}

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

//! A collision query.
/**
A collision query finds all scene-nodes colliding with some kind of object.
The type of objects depends on the query.
All queries are posed in global coordinates.
*/
class Query
{
public:
	//! The kind of data to retrieve from the collision.
	enum class EQueryLevel
	{
		//! Retrieve exact collision data
		/**
		If collision data is available, a QueryCallback::OnCollision is called for each collision,
		if no collision is avaible QueryCallback::OnObject is called.
		*/
		Collision,

		//! Just the collided object.
		/**
		For each object fullfilling the query QueryCallback::OnObject is called.
		*/
		Object,
	};

public:
	Query() {}
	LUX_API Query(Node* rootNode);
	virtual ~Query() {}

	//! Execute the query.
	/**
	\param callback The callback which is called for each collision, must be the implementation matching the query-type.
	See QueryCallback for more information.
	*/
	LUX_API virtual bool Execute(QueryCallback* callback);

	virtual core::Name GetType() const = 0;

	//! Get the precision level of the query.
	EQueryLevel GetLevel() const
	{
		return m_Level;
	}

	//! Set the precision level of the query.
	void SetLevel(EQueryLevel level)
	{
		m_Level = level;
	}

	//! Get the filter tags.
	u32 GetTags() const
	{
		return m_Tags;
	}

	//! Set the filter tags for the query.
	/**
	Only scene nodes fitting these tags are returned.
	Set to 0 to disable filtering.
	*/
	void SetTags(u32 tags)
	{
		m_Tags = tags;
	}

protected:
	WeakRef<Node> m_QueryRootNode;
	EQueryLevel m_Level;

	u32 m_Tags;
};

}
}

#endif // #ifndef INCLUDED_QUERY_H