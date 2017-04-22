#ifndef INCLUDED_QUERY_H
#define INCLUDED_QUERY_H
#include "scene/query/QueryCallback.h"
#include "core/lxName.h"

namespace lux
{
namespace scene
{
class SceneNode;
class Collider;

class Query : public ReferenceCounted
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
	LUX_API Query(SceneNode* rootNode);

	//! Execute the query.
	/**
	\param callback The callback which is called for each collision, must be the implementation matching the query-type.
	See QueryCallback for more information.
	*/
	LUX_API virtual bool Execute(QueryCallback* callback);

	virtual core::Name GetType() const = 0;

	EQueryLevel GetLevel() const
	{
		return m_Level;
	}

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
	WeakRef<SceneNode> m_QueryRootNode;
	EQueryLevel m_Level;

	u32 m_Tags;
};

}
}

#endif // #ifndef INCLUDED_QUERY_H