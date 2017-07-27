#ifndef INCLUDED_LINE_QUERY_H
#define INCLUDED_LINE_QUERY_H
#include "scene/query/Query.h"
#include "scene/query/QueryCallback.h"
#include "math/line3d.h"

namespace lux
{
namespace scene
{

//! A line query
/**
Returns all objects intersecting a given line.
*/
class LineQuery : public Query
{
public:
	LineQuery(Node* rootNode, const math::Line3F& line = math::Line3F()) :
		Query(rootNode),
		m_Line(line)
	{
	}

	core::Name GetType() const
	{
		static const core::Name name = "line";
		return name;
	}

	const math::Line3F& GetLine() const
	{
		return m_Line;
	}

	void SetLine(const math::Line3F& line)
	{
		m_Line = line;
	}

private:
	math::Line3F m_Line;
};

//! A single result of a line query.
struct LineQueryResult : QueryResult
{
	math::Vector3F position; //! Position of the collision, if there are multiple collisions, the collision next to the start of the line, in Worldcoordinates.
	math::Vector3F normal; //! Maybe the null-vector if not available, in Worldcoordinates.
	float distance; //! Line parameter where the collision occured
};

//! The result callback for a line query.
class LineQueryCallback : public QueryCallback
{
public:
	//! Called for each "real" collision.
	/**
	Will be called for each object in a EQueryLevel::Collision query.
	Defaults to all call to the OnObject method.
	*/
	virtual bool OnCollision(Node* node, const LineQueryResult& result)
	{
		LUX_UNUSED(result);
		return OnObject(node, result);
	}
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LINE_QUERY_H