#include "scene/SceneManager.h"
#include "scene/Node.h"
#include "scene/query/Query.h"
#include "scene/query/QueryCallback.h"
#include "scene/query/VolumeQuery.h"
#include "scene/collider/Collider.h"
#include "scene/components/Camera.h"
#include "scene/components/Light.h"
#include "core/Logger.h"
#include "core/ReferableRegister.h"

namespace lux
{
namespace scene
{

Node::Node(SceneManager* creator, bool isRoot) :
	m_Parent(nullptr),
	m_Sibling(nullptr),
	m_Child(nullptr),
	m_Tags(0),
	m_DebugFlags(EDD_NONE),
	m_AnimatedCount(0),
	m_SceneManager(creator),
	m_IsVisible(true),
	m_HasUserBoundingBox(false),
	m_IsRoot(isRoot)
{
	UpdateAbsTransform();
}

Node::~Node()
{
	RemoveAllChildren();
	RemoveAllComponents();
}

void Node::VisitRenderables(RenderableVisitor* visitor, bool noDebug)
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		if(!it->markForDelete)
			it->comp->VisitRenderables(visitor, noDebug);
	}
}

void Node::Animate(float time)
{
	// Animate the node, i.e. its components
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		Component* component = it->comp;
		if(!it->markForDelete) {
			if(component->IsAnimated())
				component->Animate(this, time);
		}
	}
}

void Node::CleanDeletionQueue()
{
	auto entry = m_Components.First();
	while(entry != m_Components.End()) {
		SceneNodeComponentList::Iterator next_entry;
		if(entry->markForDelete) {
			next_entry = m_Components.Erase(entry);
		} else {
			next_entry = entry;
			++next_entry;
		}

		entry = next_entry;
	}
}

StrongRef<Component> Node::AddComponent(Component* component)
{
	LX_CHECK_NULL_ARG(component);

	m_Components.PushBack(ComponentEntry(component));

	OnAddComponent(component);

	return component;
}

Node::ComponentIterator Node::GetComponentsFirst()
{
	return ComponentIterator(m_Components.First());
}

Node::ComponentIterator Node::GetComponentsEnd()
{
	return ComponentIterator(m_Components.End());
}

Node::ConstComponentIterator Node::GetComponentsFirst() const
{
	return ConstComponentIterator(m_Components.First());
}

Node::ConstComponentIterator Node::GetComponentsEnd() const
{
	return ConstComponentIterator(m_Components.End());
}

bool Node::HasComponents() const
{
	return !m_Components.IsEmpty();
}

void Node::RemoveComponent(Component* comp)
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		if(it->comp == comp) {
			m_Components.Erase(it);
			OnRemoveComponent(comp);
			break;
		}
	}
}

void Node::RemoveAllComponents()
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it)
		OnRemoveComponent(it->comp);

	m_Components.Clear();
}

void Node::AddTag(u32 tag)
{
	m_Tags |= tag;
}

void Node::RemoveTag(u32 tag)
{
	m_Tags &= ~tag;
}

bool Node::HasTag(u32 tag) const
{
	return (tag == 0 || ((m_Tags&tag) == tag));
}

void Node::SetRelativeTransform(const math::Transformation& t)
{
	m_RelativeTrans = t;
	Transformable::SetDirty();
}

const math::Transformation& Node::GetAbsoluteTransform() const
{
	UpdateAbsTransform();
	return m_AbsoluteTrans;
}

const math::Transformation& Node::GetRelativeTransform() const
{
	return m_RelativeTrans;
}

bool Node::UpdateAbsTransform() const
{
	if(!Transformable::IsDirty())
		return false;

	Node* parent = GetParent();
	if(parent && !parent->m_IsRoot) {
		m_AbsoluteTrans = parent->GetAbsoluteTransform().CombineLeft(m_RelativeTrans);
	} else {
		m_AbsoluteTrans = m_RelativeTrans;
	}

	Transformable::ClearDirty();
	return true;
}

bool Node::IsVisible() const
{
	return m_IsVisible;
}

bool Node::IsTrulyVisible() const
{
	const Node* node = this;
	while(node) {
		if(node->m_IsRoot)
			return true;

		if(node->IsVisible() == false)
			return false;

		node = node->GetParent();
	}

	return false;
}

void Node::SetVisible(bool visible)
{
	m_IsVisible = visible;
}

void Node::SwitchVisible()
{
	SetVisible(IsVisible());
}

StrongRef<Node> Node::AddChild(Node* child)
{
	LX_CHECK_NULL_ARG(child);
	if(child == this)
		throw core::InvalidArgumentException("child", "Child may not be the same node");

	if(m_SceneManager != child->m_SceneManager)
		throw core::InvalidArgumentException("child", "Child was created in diffrent scene manager");

	child->Grab();

	if(child->m_Parent)
		child->m_Parent->RemoveChild(child);

	if(m_Child) {
		Node* tmp = m_Child;
		m_Child = child;
		child->m_Sibling = tmp;
	} else {
		m_Child = child;
	}

	child->m_Parent = this;

	child->OnAttach();

	return child;
}

StrongRef<Node> Node::AddChild()
{
	return AddChild(LUX_NEW(Node(m_SceneManager)));
}

bool Node::HasChildren() const
{
	return (m_Child != nullptr);
}

void Node::RemoveChild(Node* child)
{
	LX_CHECK_NULL_ARG(child);

	Node* last = nullptr;
	Node* current = m_Child;
	while(current) { // Iterator over each child
		if(current == child) {
			child->OnDettach(); // Send dettach event

			if(last)
				last->m_Sibling = current->m_Sibling;
			else
				m_Child = current->m_Sibling;

			child->m_Parent = nullptr;
			child->m_Sibling = nullptr;

			child->Drop(); // Delete child node
			return;
		}

		last = current;
		current = current->m_Sibling;
	}
}

void Node::RemoveAllChildren()
{
	if(!m_Child)
		return;

	Node* current = m_Child;
	Node* next;
	while(current) {
		current->OnDettach();

		next = current->m_Sibling;

		current->m_Parent = nullptr;
		current->m_Sibling = nullptr;

		current->Drop();

		current = next;
	}

	m_Child = nullptr;
}

void Node::Remove()
{
	Node* parent = GetParent();
	if(parent)
		parent->RemoveChild(this);
}

Node::ChildIterator Node::GetChildrenFirst()
{
	return ChildIterator(m_Child);
}

Node::ChildIterator Node::GetChildrenEnd()
{
	return ChildIterator(nullptr);
}

Node::ConstChildIterator Node::GetChildrenFirst() const
{
	return ConstChildIterator(m_Child);
}

Node::ConstChildIterator Node::GetChildrenEnd() const
{
	return ConstChildIterator(nullptr);
}

void Node::MarkForDelete()
{
	if(m_SceneManager)
		m_SceneManager->AddToDeletionQueue(this);
}

void Node::MarkForDelete(Component* comp)
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		if(it->comp == comp) {
			OnRemoveComponent(comp);
			it->markForDelete = true;
			break;
		}
	}
}

void Node::SetParent(Node* newParent)
{
	if(newParent == GetParent())
		return;

	newParent->AddChild(this);
	Transformable::SetDirty();
}

Node* Node::GetParent() const
{
	return m_Parent;
}

SceneManager* Node::GetSceneManager() const
{
	return m_SceneManager;
}

Node* Node::GetRoot()
{
	if(m_SceneManager)
		return m_SceneManager->GetRoot();
	return this;
}

void Node::SetDebugData(EDebugData debugData, bool state)
{
	if(state)
		m_DebugFlags |= debugData;
	else
		m_DebugFlags &= ~debugData;
}

bool Node::GetDebugData(EDebugData debugData)
{
	return (m_DebugFlags & debugData) != 0;
}

StrongRef<Collider> Node::SetCollider(Collider* collider)
{
	m_Collider = collider;
	return m_Collider;
}

StrongRef<Collider> Node::GetCollider() const
{
	return m_Collider;
}

bool Node::ExecuteQuery(Query* query, QueryCallback* callback)
{
	bool wasNotAborted = true;
	if(HasTag(query->GetTags()) && m_Collider)
		wasNotAborted = m_Collider->ExecuteQuery(this, query, callback);

	for(auto it = GetChildrenFirst(); wasNotAborted && it != GetChildrenEnd(); ++it)
		wasNotAborted = it->ExecuteQuery(query, callback);

	return wasNotAborted;
}

const math::aabbox3df& Node::GetBoundingBox() const
{
	return m_BoundingBox;
}

void Node::SetBoundingBox(const math::aabbox3df& box)
{
	m_BoundingBox = box;
	m_HasUserBoundingBox = true;
}

void Node::RecalculateBoundingBox()
{
	class BoxVisitor : public RenderableVisitor
	{
	public:
		void Visit(Renderable* r)
		{
			if(!r->GetBoundingBox().IsEmpty()) {
				if(box.IsEmpty())
					box = r->GetBoundingBox();
				else
					box.AddBox(r->GetBoundingBox());
			}
		}

		math::aabbox3df box = math::aabbox3df::EMPTY;
	};

	BoxVisitor visitor;
	VisitRenderables(&visitor, true);

	m_BoundingBox = visitor.box;
	m_HasUserBoundingBox = false;
}

StrongRef<Referable> Node::Clone() const
{
	return LUX_NEW(Node)(*this);
}

core::Name Node::GetReferableType() const
{
	static const core::Name name = "lux.node";
	return name;
}

Node::Node(const Node& other) :
	m_Parent(nullptr),
	m_Sibling(nullptr),
	m_Child(nullptr),
	m_Tags(other.m_Tags),
	m_DebugFlags(other.m_DebugFlags),
	m_IsVisible(other.m_IsVisible),
	m_SceneManager(other.m_SceneManager)
{
	for(auto it = GetChildrenFirst(); it != GetChildrenEnd(); ++it) {
		StrongRef<Node> node = (*it)->Clone();
		node->SetParent(this);
	}

	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		StrongRef<Component> comp = it->comp->Clone();
		AddComponent(comp);
	}
}

void Node::OnAttach()
{
}

void Node::OnDettach()
{
}

void Node::OnAddComponent(Component* c)
{
	if(m_SceneManager) {
		auto camera = dynamic_cast<Camera*>(c);
		if(camera)
			m_SceneManager->RegisterCamera(this, camera);

		auto light = dynamic_cast<Light*>(c);
		if(light)
			m_SceneManager->RegisterLight(this, light);

		if(c->IsAnimated()) {
			if(m_AnimatedCount == 0)
				m_SceneManager->RegisterAnimated(this);
			++m_AnimatedCount;
		}
	}

	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnAttach(this);
}

void Node::OnRemoveComponent(Component* c)
{
	if(m_SceneManager) {
		auto camera = dynamic_cast<Camera*>(c);
		if(camera)
			m_SceneManager->UnregisterCamera(this, camera);

		auto light = dynamic_cast<Light*>(c);
		if(light)
			m_SceneManager->UnregisterLight(this, light);
		if(c->IsAnimated()) {
			--m_AnimatedCount;
			if(m_AnimatedCount == 0)
				m_SceneManager->UnregisterAnimated(this);
		}
	}

	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnDettach(this);
}

void Node::SetDirty() const
{
	if(IsDirty())
		return;

	Transformable::SetDirty();

	// Set all children dirty to
	for(auto it = GetChildrenFirst(); it != GetChildrenEnd(); ++it)
		it->SetDirty();
}

} // namespace scene
} // namespace lux
