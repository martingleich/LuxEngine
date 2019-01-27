#include "scene/Scene.h"

#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"

#include "scene/Node.h"

namespace lux
{
namespace scene
{

////////////////////////////////////////////////////////////////////////////////////

Scene::Scene()
{
	m_Root = LUX_NEW(Node)(this);
}

Scene::~Scene()
{
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AddToDeletionQueue(Node* node)
{
	if(node)
		m_NodeDeletionQueue.PushBack(node);
}
void Scene::AddToDeletionQueue(Component* comp)
{
	if(comp)
		m_CompDeletionQueue.PushBack(comp);
}

void Scene::ClearDeletionQueue()
{
	for(auto n : m_NodeDeletionQueue) {
		auto p = n->GetParent();
		if(p)
			p->RemoveChild(n);
	}
	m_NodeDeletionQueue.Clear();
	for(auto c : m_CompDeletionQueue) {
		auto p = c->GetNode();
		if(p)
			p->RemoveComponent(c);
	}
	m_CompDeletionQueue.Clear();
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Node> Scene::AddNode(Component* baseComp, Node* parent)
{
	if(!parent)
		parent = GetRoot();

	auto node = parent->AddChild();
	if(baseComp)
		node->AddComponent(baseComp);

	return node;
}

////////////////////////////////////////////////////////////////////////////////////

Node* Scene::GetRoot() const
{
	return m_Root;
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterForTick(Component* c, bool doRegister)
{
	if(doRegister)
		m_AnimatedComps.Insert(c);
	else
		m_AnimatedComps.Erase(c);
}
void Scene::RegisterLight(Component* c, bool doRegister)
{
	if(doRegister)
		m_LightComps.Insert(c);
	else
		m_LightComps.Erase(c);
}

void Scene::RegisterFog(Component* c, bool doRegister)
{
	if(doRegister)
		m_FogComps.Insert(c);
	else
		m_FogComps.Erase(c);
}

void Scene::RegisterCamera(Component* c, bool doRegister)
{
	if(doRegister)
		m_CamComps.Insert(c);
	else
		m_CamComps.Erase(c);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto comp : m_AnimatedComps)
		comp->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

static void VisitComponentsRec(
	Node* node, ComponentVisitor* visitor)
{
	for(auto c : node->Components())
		visitor->Visit(node, c);
	if(visitor->ShouldAbortChildren()) {
		visitor->ResumeChildren();
		return;
	}
	for(auto child : node->Children())
		VisitComponentsRec(child, visitor);
}

void Scene::VisitComponents(
	ComponentVisitor* visitor,
	Node* root)
{
	if(!root)
		root = m_Root;
	VisitComponentsRec(root, visitor);
}

} // namespace scene
} // namespace lux