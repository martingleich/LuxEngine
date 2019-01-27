#include "scene/Scene.h"
#include "scene/Node.h"
#include "scene/Renderable.h"
#include "scene/Collider.h"
#include "core/Logger.h"
#include "scene/Component.h"

namespace lux
{
namespace scene
{

Node::Node(Scene* scene) :
	m_Parent(nullptr),
	m_Sibling(nullptr),
	m_Child(nullptr),
	m_Tags(0),
	m_Scene(scene),
	m_IsVisible(true),
	m_HasUserBoundingBox(false),
	m_CastShadow(true),
	m_InheritTranslation(true),
	m_InheritRotation(true),
	m_InheritScale(true),
	m_IsTransformDirty(true)
{
	ConditionalUpdateAbsTransform();
}

Node::~Node()
{
	RemoveAllChildren();
	RemoveAllComponents();
}

void Node::Animate(float time)
{
	for(auto& c : m_Components) {
		if(c->IsAnimated())
			c->Animate(time);
	}
}

StrongRef<Component> Node::AddComponent(Component* component)
{
	LX_CHECK_NULL_ARG(component);

	m_Components.PushBack(component);
	OnAttach(component);
	return component;
}

core::Range<Node::ComponentIterator> Node::Components()
{
	return core::Range<Node::ComponentIterator>(
		ComponentIterator(m_Components.First()),
		ComponentIterator(m_Components.End()));
}

bool Node::HasComponents()
{
	return !m_Components.IsEmpty();
}

void Node::RemoveComponent(Component* comp)
{
	for(int i = 0; i < m_Components.Size(); ++i) {
		if(m_Components[i] == comp) {
			m_Components.Erase(i);
			OnDetach(comp);
			break;
		}
	}
}

void Node::RemoveAllComponents()
{
	for(auto& c : m_Components)
		OnDetach(c);

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
	SetTransformDirty();
}

const math::Transformation& Node::GetAbsoluteTransform()
{
	ConditionalUpdateAbsTransform();
	return m_AbsoluteTrans;
}

void Node::ConditionalUpdateAbsTransform()
{
	if(!IsTransformDirty())
		return;
	UpdateAbsTransform();
}

void Node::UpdateAbsTransform()
{
	Node* parent = GetParent();
	if(parent && !parent->IsRoot()) {
		auto& pt = parent->GetAbsoluteTransform();
		math::Vector3F translation = m_RelativeTrans.translation;
		if(IsInheritingScale()) {
			m_AbsoluteTrans.scale = m_RelativeTrans.scale * pt.scale;
			translation *= pt.scale;
		} else {
			m_AbsoluteTrans.scale = m_RelativeTrans.scale;
		}
		if(IsInheritingRotation()) {
			m_AbsoluteTrans.orientation = m_RelativeTrans.orientation * pt.orientation;
			pt.orientation.TransformInPlace(translation);
		} else {
			m_AbsoluteTrans.orientation = m_RelativeTrans.orientation;
		}
		if(IsInheritingTranslation()) {
			m_AbsoluteTrans.translation = translation + pt.translation;
		} else {
			m_AbsoluteTrans.translation = translation;
		}
	} else {
		m_AbsoluteTrans = m_RelativeTrans;
	}

	ClearTransformDirty();
}

bool Node::IsVisible() const
{
	return m_IsVisible;
}

bool Node::IsTrulyVisible() const
{
	return m_IsTrulyVisible;
}

void Node::SetVisible(bool visible)
{
	if(m_IsVisible != visible) {
		m_IsVisible = visible;
		UpdateTrulyVisible();
	}
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

core::Range<Node::ChildIterator> Node::Children()
{
	return core::Range<Node::ChildIterator>(
		ChildIterator(m_Child), ChildIterator(nullptr));
}

void Node::MarkForDelete()
{
	if(m_Scene)
		m_Scene->AddToDeletionQueue(this);
	else
		Remove();
}

void Node::MarkForDelete(Component* comp)
{
	if(m_Scene)
		m_Scene->AddToDeletionQueue(comp);
	else
		RemoveComponent(comp);
}

void Node::SetParent(Node* newParent)
{
	if(newParent == GetParent())
		return;

	newParent->AddChild(this);
	SetTransformDirty();
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

struct BoundingBoxCollector
{
	void Add(const math::AABBoxF& b)
	{
		if(box.IsEmpty())
			box = b;
		else if(!b.IsEmpty())
			box.AddBox(b);
	}

	math::AABBoxF box = math::AABBoxF::EMPTY;
};

void Node::RecalculateBoundingBox()
{
	BoundingBoxCollector boxCol;
	for(auto& c : m_Components)
		boxCol.Add(c->GetBoundingBox());
	m_BoundingBox = boxCol.box;
	m_HasUserBoundingBox = false;
}

StrongRef<Node> Node::Clone() const
{
	return static_cast<Node*>(CloneImpl().Raw());
}

StrongRef<core::Referable> Node::CloneImpl() const
{
	return LUX_NEW(Node)(*this);
}

void Node::UpdateTrulyVisible(bool parentVisible)
{
	auto newVisiblity = m_IsVisible && parentVisible;
	if(newVisiblity != m_IsTrulyVisible) {
		m_IsTrulyVisible = newVisiblity;
		for(auto c : Children())
			c->UpdateTrulyVisible(m_IsTrulyVisible);
	}
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
	m_Scene(other.m_Scene),
	m_IsVisible(other.m_IsVisible),
	m_HasUserBoundingBox(false),
	m_CastShadow(true)
{
	for(auto child : Children()) {
		StrongRef<Node> node = child->Clone();
		node->SetParent(this);
	}

	for(auto& c : m_Components)
		AddComponent(c->Clone());
}

void Node::OnAttach()
{
	Register(true);
	UpdateTrulyVisible();
}

void Node::OnDetach()
{
	Register(false);
	UpdateTrulyVisible();
}

void Node::OnAttach(Component* c)
{
	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnAttach(this);
	c->Register(true);
}

void Node::OnDetach(Component* c)
{
	if(!m_HasUserBoundingBox)
		RecalculateBoundingBox();

	c->OnDetach(this);
	c->Register(false);
}

void Node::SetTransformDirty()
{
	if(IsTransformDirty())
		return;

	m_IsTransformDirty = true;

	// Set all children dirty to
	for(auto child : Children())
		child->SetTransformDirty();
}

void Node::Register(bool doRegister)
{
	for(auto c : Components())
		c->Register(doRegister);
	for(auto c : Children())
		c->Register(doRegister);
}

} // namespace scene
} // namespace lux
