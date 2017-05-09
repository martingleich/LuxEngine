#include "scene/query/Query.h"
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

Query::Query(SceneNode* rootNode) :
	m_QueryRootNode(rootNode),
	m_Level(EQueryLevel::Object),
	m_Tags(0)
{
}

bool Query::Execute(QueryCallback* callback)
{
	return m_QueryRootNode->ExecuteQuery(this, callback);
}

}
}