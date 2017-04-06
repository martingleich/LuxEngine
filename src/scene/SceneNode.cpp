#include "scene/SceneManager.h"
#include "scene/SceneNode.h"
#include "scene/Query.h"
#include "scene/QueryCallback.h"

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

EResult SceneNode::ExecuteQuery(Query* query, QueryCallback* callback)
{
	auto result = EResult::Succeeded;
	if(HasTag(query->GetTags()))
		result = m_Collider->ExecuteQuery(this, query, callback);
	for(auto it = GetChildrenFirst(); result != EResult::Aborted && it != GetChildrenEnd(); ++it) {
		result = it->ExecuteQuery(query, callback);
	}

	if(result == EResult::Aborted)
		return EResult::Aborted;
	return EResult::Succeeded;
}

}
}