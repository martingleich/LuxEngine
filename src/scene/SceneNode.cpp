#include "scene/SceneManager.h"
#include "scene/SceneNode.h"
#include "scene/query/Query.h"
#include "scene/query/QueryCallback.h"
#include "scene/query/VolumeQuery.h"
#include "core/Logger.h"

namespace lux
{
namespace scene
{

namespace SceneNodeType
{
const core::Name SkyBox = "skybox";
const core::Name Root = "root";
const core::Name Camera = "camera";
const core::Name Mesh = "mesh";
const core::Name Light = "light";
const core::Name Empty = "empty";
}

void SceneNode::SetEventReceiver(bool Receive)
{
	if(m_IsEventReceiver == Receive)
		return;

	if(m_SceneManager) {
		if(Receive)
			m_SceneManager->RegisterEventReceiver(this);
		else
			m_SceneManager->UnregisterEventReceiver(this);
	}

	m_IsEventReceiver = Receive;
}

void SceneNode::MarkForDelete()
{
	if(m_SceneManager)
		m_SceneManager->AddToDeletionQueue(this);
}

SceneNode* SceneNode::GetRoot()
{
	if(m_SceneManager)
		return m_SceneManager->GetRootSceneNode();
	return this;
}

bool SceneNode::ExecuteQuery(Query* query, QueryCallback* callback)
{
	bool wasNotAborted = true;
	if(HasTag(query->GetTags()) && m_Collider)
		wasNotAborted = m_Collider->ExecuteQuery(this, query, callback);

	for(auto it = GetChildrenFirst(); wasNotAborted && it != GetChildrenEnd(); ++it)
		wasNotAborted = it->ExecuteQuery(query, callback);

	return wasNotAborted;
}

}
}