#include "scene/Scene.h"
#include "scene/Node.h"
#include "scene/Renderable.h"
#include "scene/Collider.h"
#include "core/Logger.h"

namespace lux
{
namespace scene
{

Node::Node(Scene* scene, bool isRoot) :
	m_Parent(nullptr),
	m_Sibling(nullptr),
	m_Child(nullptr),
	m_Tags(0),
	m_AnimatedCount(0),
	m_Scene(scene),
	m_IsVisible(true),
	m_IsRoot(isRoot),
	m_HasUserBoundingBox(false),
	m_CastShadow(true),
	m_IsDirty(true)
{
	UpdateAbsTransform();
}

Node::~Node()
{
	RemoveAllChildren();
	RemoveAllComponents();
}

void Node::VisitRenderables(RenderableVisitor* visitor, ERenderableTags tags)
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		if(!it->markForDelete)
			it->comp->VisitRenderables(visitor, tags);
	}
}

void Node::Animate(float time)
{
	// Animate the node, i.e. its components
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		Component* component = it->comp;
		if(!it->markForDelete) {
			if(component->IsAnimated())
				component->Animate(time);
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

	OnAttach(component);

	return component;
}

core::Range<Node::ComponentIterator> Node::Components()
{
	return  core::Range<Node::ComponentIterator>(
		ComponentIterator(m_Components.First()),
		ComponentIterator(m_Components.End()));
}

core::Range<Node::ConstComponentIterator> Node::Components() const
{
	return  core::Range<Node::ConstComponentIterator>(
		ConstComponentIterator(m_Components.First()),
		ConstComponentIterator(m_Components.End()));
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
			OnDetach(comp);
			break;
		}
	}
}

void Node::RemoveAllComponents()
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it)
		OnDetach(it->comp);

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
	SetDirty();
}

const math::Transformation& Node::GetAbsoluteTransform() const
{
	UpdateAbsTransform();
	return m_AbsoluteTrans;
}

bool Node::UpdateAbsTransform() const
{
	if(!IsDirty())
		return false;

	Node* parent = GetParent();
	if(parent && !parent->m_IsRoot)
		m_AbsoluteTrans = parent->GetAbsoluteTransform().CombineLeft(m_RelativeTrans);
	else
		m_AbsoluteTrans = m_RelativeTrans;

	ClearDirty();
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
		throw core::GenericInvalidArgumentException("child", "Child may not be the same node");

	if(m_Scene != child->m_Scene)
		throw core::GenericInvalidArgumentException("child", "Child was created in diffrent scene manager");

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
	return AddChild(LUX_NEW(Node(m_Scene)));
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
			child->OnDetach(); // Send dettach event

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
		current->OnDetach();

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

core::Range<Node::ChildIterator> Node::Children()
{
	return core::Range<Node::ChildIterator>(
		ChildIterator(m_Child), ChildIterator(nullptr));
}

core::Range<Node::ConstChildIterator> Node::Children() const
{
	return core::Range<Node::ConstChildIterator>(
		ConstChildIterator(m_Child), ConstChildIterator(nullptr));
}

void Node::MarkForDelete()
{
	if(m_Scene)
		m_Scene->AddToDeletionQueue(this);
}

void Node::MarkForDelete(Component* comp)
{
	for(auto it = m_Components.First(); it != m_Components.End(); ++it) {
		if(it->comp == comp) {
			OnAttach(comp);
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
	SetDirty();
}

Node* Node::GetParent() const
{
	return m_Parent;
}

Scene* Node::GetScene() const
{
	return m_Scene;
}

Node* Node::GetRoot()
{
	if(m_Scene)
		return m_Scene->GetRoot();
	return this;
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

const math::AABBoxF& Node::GetBoundingBox() const
{
	return m_BoundingBox;
}

void Node::SetBoundingBox(const math::AABBoxF& box)
{
	m_BoundingBox = box;
	m_HasUserBoundingBox = true;
}

void Node::RecalculateBoundingBox()
{
	class BoxVisitor : public RenderableVisitor
	{
	public:
		void Visit(Node*, Renderable* r)
		{
			if(!r->GetBoundingBox().IsEmpty()) {
				if(box.IsEmpty())
					box = r->GetBoundingBox();
				else
					box.AddBox(r->GetBoundingBox());
			}
		}

		math::AABBoxF box = math::AABBoxF::EMPTY;
	};

	BoxVisitor visitor;
	VisitRenderables(&visitor, ERenderableTags::None);

	m_BoundingBox = visitor.box;
	m_HasUserBoundingBox = false;
}

StrongRef<Node> Node::Clone() const
{
	return static_cast<Node*>(CloneImpl().Raw());
}

StrongRef<Referable> Node::CloneImpl() const
{
	return LUX_NEW(Node)(*this);
}

core::Name Node::GetReferableType() const
{
	static const core::Name name("lux.node");
	return name;
}

Node::Node(const Node& other) :
	m_Parent(nullptr),
	m_Sibling(nullptr),
	m_Child(nullptr),
	m_Tags(other.m_Tags),
	m_AnimatedCount(0),
	m_Scene(other.m_Scene),
	m_IsVisible(other.m_IsVisible),
	m_HasUserBoundingBox(false),
	m_IsRoot(true),
	m_CastShadow(true)
{
	for(auto child : Children()) {
		StrongRef<Node> node = child->Clone();
		node->SetParent(this);
	}

	for(auto& ce : m_Components) {
		StrongRef<Component> comp = ce.comp->Clone();
		AddComponent(comp);
	}
}

void Node::OnAttach()
{
	if(m_Scene)
		m_Scene->OnAttach(this);
}

void Node::OnDetach()
{
	if(m_Scene)
		m_Scene->OnDetach(this);
}

void Node::OnAttach(Component* c)
{
	if(m_Scene) {
		m_Scene->OnAttach(c);

		if(c->IsAnimated()) {
			if(m_AnimatedCount == 0)
				m_Scene->RegisterAnimated(this);
			++m_AnimatedCount;
		}
	}

	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnAttach(this);
}

void Node::OnDetach(Component* c)
{
	if(m_Scene) {
		m_Scene->OnDetach(c);

		if(c->IsAnimated()) {
			--m_AnimatedCount;
			if(m_AnimatedCount == 0)
				m_Scene->UnregisterAnimated(this);
		}
	}

	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnDetach(this);
}

void Node::SetDirty() const
{
	if(IsDirty())
		return;

	m_IsDirty = true;

	// Set all children dirty to
	for(auto child : Children())
		child->SetDirty();
}

bool Node::IsShadowCasting() const
{
	return m_CastShadow;
}

void Node::SetShadowCasting(bool cast)
{
	m_CastShadow = cast;
}

} // namespace scene
} // namespace lux
