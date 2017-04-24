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

EResult SceneNode::ExecuteQuery(Query* query, QueryCallback* callback)
{
	auto result = EResult::Succeeded;
	if(HasTag(query->GetTags()) && m_Collider) {
		result = m_Collider->ExecuteQuery(this, query, callback);
		if(result == EResult::NotImplemented) {
			bool isVolume = (query->GetType() == "volume");
			const char* levelString;
			if(query->GetLevel() == Query::EQueryLevel::Collision)
				levelString = "collision";
			else if(query->GetLevel() == Query::EQueryLevel::Object)
				levelString = "object";
			else
				levelString = "unknown";
			if(!isVolume)
				log::Warning("Performed not implemented query(level: ~a, query: ~a, collider: ~a).", levelString, query->GetType(), m_Collider->GetReferableSubType());
			else {
				VolumeQuery* vq = (VolumeQuery*)query;
				log::Warning("Performed not implemented query(level: ~a, query: ~a, zone: ~a, collider: ~a).", levelString, query->GetType(), vq->GetZone()->GetReferableSubType() , m_Collider->GetReferableSubType());
			}
		}
	}

	for(auto it = GetChildrenFirst(); result != EResult::Aborted && it != GetChildrenEnd(); ++it) {
		result = it->ExecuteQuery(query, callback);
	}

	if(result == EResult::Aborted)
		return EResult::Aborted;
	return EResult::Succeeded;
}

}
}