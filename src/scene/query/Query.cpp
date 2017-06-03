#include "scene/query/Query.h"
#include "scene/Node.h"

namespace lux
{
namespace scene
{

Query::Query(Node* rootNode) :
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