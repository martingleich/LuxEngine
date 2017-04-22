#ifndef INCLUDED_LINE_QUERY_H
#define INCLUDED_LINE_QUERY_H
#include "scene/query/Query.h"
#include "math/line3d.h"

namespace lux
{
namespace scene
{

class LineQuery : public Query
{
public:
	LineQuery(SceneNode* rootNode, const math::line3df& line = math::line3df()) :
		Query(rootNode),
		m_Line(line)
	{
	}

	core::Name GetType() const
	{
		static const core::Name name = "line";
		return name;
	}

	const math::line3df& GetLine() const
	{
		return m_Line;
	}

	void SetLine(const math::line3df& line)
	{
		m_Line = line;
	}

private:
	math::line3df m_Line;
};

//! A single result of a line query.
struct LineQueryResult : QueryResult
{
	math::vector3f position; //! Position of the collision, if there are multiple collisions, the collision next to the start of the line, in Worldcoordinates.
	math::vector3f normal; //! Maybe the null-vector if not available, in Worldcoordinates.
	float distance; //! Line parameter where the collision occured
};

class LineQueryCallback : public QueryCallback
{
public:
	virtual bool OnCollision(SceneNode* node, const LineQueryResult& result)
	{
		LUX_UNUSED(result);
		return OnObject(node);
	}
};

}
}

#endif // #ifndef INCLUDED_LINE_QUERY_H