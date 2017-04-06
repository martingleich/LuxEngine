#include "scene/Query.h"
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

Query::Query(SceneNode* rootNode) :
	m_QueryRootNode(rootNode),
	m_Level(EQueryLevel::Object)
{
}

bool Query::Execute(QueryCallback* callback)
{
	auto result = m_QueryRootNode->ExecuteQuery(this, callback);
	return Succeeded(result);
}

}
}