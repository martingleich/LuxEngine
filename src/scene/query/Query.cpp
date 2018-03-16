#include "scene/Query.h"
#include "scene/Node.h"
#include "scene/Collider.h"

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

static bool QueryExecuteRec(Query* query, QueryCallback* callback, Node* node)
{
	bool wasNotAborted = true;
	if(node->HasTag(query->GetTags()) && node->GetCollider())
		wasNotAborted = node->GetCollider()->ExecuteQuery(node, query, callback);

	for(auto child : node->Children()) {
		wasNotAborted = QueryExecuteRec(query, callback, child);
		if(!wasNotAborted)
			break;
	}

	return wasNotAborted;
}

bool Query::Execute(QueryCallback* callback)
{
	return QueryExecuteRec(this, callback, m_QueryRootNode);
}

}
}