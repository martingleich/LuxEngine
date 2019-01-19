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
	m_Root = LUX_NEW(Node)(this, true);
}

Scene::~Scene()
{
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterObserver(SceneObserver* observer)
{
	m_Observers.PushBack(observer);
}

void Scene::UnregisterObserver(SceneObserver* observer)
{
	auto i = m_Observers.LinearSearch(observer);
	if(i != -1)
		m_Observers.Erase(i);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::Clear()
{
	ClearDeletionQueue();

	m_AnimatedNodes.Clear();

	m_Root->RemoveAllChildren();
}

void Scene::AddToDeletionQueue(Node* node)
{
	if(node)
		m_DeletionQueue.PushBack(node);
}

void Scene::ClearDeletionQueue()
{
	for(auto it = m_DeletionQueue.First(); it != m_DeletionQueue.End(); ++it)
		(*it)->Remove();

	m_DeletionQueue.Clear();
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

bool Scene::IsEmpty() const
{
	return !GetRoot()->HasChildren();
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterAnimated(Node* node)
{
	m_AnimatedNodes.Insert(node);
}

void Scene::UnregisterAnimated(Node* node)
{
	m_AnimatedNodes.Erase(node);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto node : m_AnimatedNodes)
		node->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

static void VisitRenderablesRec(
	Node* node, RenderableVisitor* visitor,
	ERenderableTags tags)
{
	if(node->IsVisible() || TestFlag(tags, ERenderableTags::Invisible)) {
		node->VisitRenderables(visitor, tags);

		for(auto child : node->Children())
			VisitRenderablesRec(child, visitor, tags);
	}
}

void Scene::VisitRenderables(
	RenderableVisitor* visitor,
	ERenderableTags tags, Node* root)
{
	if(!root)
		root = m_Root;
	VisitRenderablesRec(root, visitor, tags);
}

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

////////////////////////////////////////////////////////////////////////////////////

void Scene::OnAttach(Node* node)
{
	for(auto& obs : m_Observers)
		obs->OnAttach(node);
}

void Scene::OnDetach(Node* node)
{
	for(auto& obs : m_Observers)
		obs->OnDetach(node);
}

void Scene::OnAttach(Component* comp)
{
	for(auto& obs : m_Observers)
		obs->OnAttach(comp);
}

void Scene::OnDetach(Component* comp)
{
	for(auto& obs : m_Observers)
		obs->OnDetach(comp);
}

} // namespace scene
} // namespace lux