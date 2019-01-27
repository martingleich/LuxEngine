#ifndef INCLUDED_LUX_SCENE_NODE_H
#define INCLUDED_LUX_SCENE_NODE_H
#include "core/lxName.h"
#include "core/lxArray.h"

#include "math/Transformation.h"
#include "scene/Component.h"

namespace lux
{
namespace scene
{
class RenderableVisitor;
class Renderable;
class Component;
class Collider;
class Scene;

class Node : public core::Referable
{
	friend class Component;
private:
	typedef core::Array<StrongRef<Component>> SceneNodeComponentList;

public:
	class ComponentIterator : core::BaseIterator<core::BidirectionalIteratorTag, Component*>
	{
	public:
		ComponentIterator() {}
		explicit ComponentIterator(typename SceneNodeComponentList::Iterator begin) : m_Current(begin) {}

		ComponentIterator& operator++() { ++m_Current; return *this; }
		ComponentIterator operator++(int) { ComponentIterator temp = *this; ++m_Current; return temp; }

		bool operator==(const ComponentIterator& other) const { return m_Current == other.m_Current; }
		bool operator!=(const ComponentIterator& other) const { return m_Current != other.m_Current; }

		Component* operator*() const { return *m_Current; }
		Component* operator->() const { return *m_Current; }

	private:
		SceneNodeComponentList::Iterator m_Current;
	};

	class ChildIterator : public core::BaseIterator<core::BidirectionalIteratorTag, Node*>
	{
	public:
		ChildIterator() {}
		explicit ChildIterator(Node* begin) : m_Current(begin) {}

		ChildIterator& operator++() { m_Current = m_Current->m_Sibling; return *this; }
		ChildIterator  operator++(int) { ChildIterator Temp = *this; m_Current = m_Current->m_Sibling; return Temp; }

		bool operator==(const ChildIterator& other) const { return m_Current == other.m_Current; }
		bool operator!=(const ChildIterator& other) const { return m_Current != other.m_Current; }
		Node* operator*() const { return m_Current; }
		Node* operator->() const { return m_Current; }

	private:
		Node* m_Current;
	};

public:
	LUX_API Node(Scene* scene);
	LUX_API ~Node();

	LUX_API void Animate(float time);

	////////////////////////////////////////////////////////////////////////////////
	//! Marks a scene node componenent for deletion.
	/**
	This method can be called at any time in the program.
	The component will be removed before the next frame.
	\param comp The component to remove.
	*/
	LUX_API void MarkForDelete(Component* comp);
	LUX_API void MarkForDelete();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Component> AddComponent(Component* component);
	LUX_API core::Range<ComponentIterator> Components();

	template <typename T>
	T* GetComponent(T* cur = nullptr)
	{
		for(auto comp : Components()) {
			T* p = dynamic_cast<T*>(comp);
			if(p && p != cur)
				return p;
		}

		return nullptr;
	}

	template <typename T>
	bool HasComponent()
	{
		return (GetComponent<T>() != nullptr);
	}

	LUX_API bool HasComponents();

	//! Removes a component from the scene node
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use Node::MarkForDelete.
	\param comp The component to remove from the node.
	\return Was the component found, and deleted.
	*/
	LUX_API void RemoveComponent(Component* comp);

	//! Removes all components
	LUX_API void RemoveAllComponents();

	////////////////////////////////////////////////////////////////////////////////
	//! Add a tag to this scene node.
	/**
	Each scene node can have up to 32 tags, each represented by a bit in an integer.
	These tags are never manipulates by the engine, and are under full user control.
	They can be used to identify groups of scene nodes.
	\param tag The tags to add to this scene node.
	*/
	LUX_API void AddTag(u32 tag);

	//! Remove tags from the scene node.
	LUX_API void RemoveTag(u32 tag);

	//! Check if all given tags are set in this scene node.
	LUX_API bool HasTag(u32 tag) const;

	////////////////////////////////////////////////////////////////////////////////

	void SetInheritScale(bool b)
	{
		if(m_InheritScale != b) {
			m_InheritScale = b;
			SetTransformDirty();
		}
	}
	void SetInheritRotation(bool b)
	{
		if(m_InheritRotation != b) {
			m_InheritRotation = b;
			SetTransformDirty();
		}
	}
	void SetInheritTranslation(bool b)
	{
		if(m_InheritTranslation != b) {
			m_InheritTranslation = b;
			SetTransformDirty();
		}
	}
	bool IsInheritingScale() const { return m_InheritScale; }
	bool IsInheritingRotation() const { return m_InheritRotation; }
	bool IsInheritingTranslation() const { return m_InheritTranslation; }

	LUX_API void SetRelativeTransform(const math::Transformation& t);
	LUX_API const math::Transformation& GetAbsoluteTransform();
	const math::Vector3F& GetAbsolutePosition()
	{
		return GetAbsoluteTransform().translation;
	}

	//! Set a new uniform scale
	/**
	\param s The new scale, must be bigger than zero
	*/
	void SetScale(float s)
	{
		lxAssert(s >= 0);
		if(s >= 0) {
			m_RelativeTrans.scale = s;
			SetTransformDirty();
		}
	}

	//! Change the scale
	/**
	\param s The change of the scale, must be bigger than zero
	*/
	void Scale(float s)
	{
		lxAssert(s >= 0);
		if(s >= 0) {
			m_RelativeTrans.scale *= s;
			SetTransformDirty();
		}
	}

	//! Set a new position
	/**
	\param p The new position
	*/
	void SetPosition(const math::Vector3F& p)
	{
		m_RelativeTrans.translation = p;
		SetTransformDirty();
	}

	void SetPosition(float x, float y, float z)
	{
		SetPosition(math::Vector3F(x, y, z));
	}

	//! Translate the node in relative coordinates
	/**
	\param p The translation
	*/
	void Translate(const math::Vector3F& p)
	{
		m_RelativeTrans.translation += p;
		SetTransformDirty();
	}

	void Translate(float x, float y, float z)
	{
		Translate(math::Vector3F(x, y, z));
	}

	//! Set a new orientation
	/**
	\param o The new orientation
	*/
	void SetOrientation(const math::QuaternionF& o)
	{
		m_RelativeTrans.orientation = o;
		SetTransformDirty();
	}

	//! Apply a rotation to the transformable
	/**
	\param o The orientation to apply.
	*/
	void Rotate(const math::QuaternionF& o)
	{
		m_RelativeTrans.orientation *= o;
		SetTransformDirty();
	}

	void Rotate(const math::Vector3F& axis, math::AngleF alpha)
	{
		Rotate(math::QuaternionF(axis, alpha));
	}

	void RotateX(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_X, alpha);
	}

	void RotateY(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_Y, alpha);
	}

	void RotateZ(math::AngleF alpha)
	{
		Rotate(math::Vector3F::UNIT_Z, alpha);
	}

	//! Get the current relative scale
	/**
	\return The current relative scale
	*/
	float GetScale()
	{
		return m_RelativeTrans.scale;
	}

	//! Get the current relative position
	/**
	\return The current relative position
	*/
	const math::Vector3F& GetPosition()
	{
		return m_RelativeTrans.translation;
	}

	//! Get the current relative orientation
	/**
	\return The current relative orientation
	*/
	const math::QuaternionF& GetOrientation()
	{
		return m_RelativeTrans.orientation;
	}

	//! Set a direction of the transfomable
	/**
	Rotates the transforable to move a local vector in direction of another vector
	\param dir The direction in which the local direction should point
	\param local The local vector which should used as pointer
	*/
	void SetDirection(const math::Vector3F& dir, const math::Vector3F& local = math::Vector3F::UNIT_Z)
	{
		m_RelativeTrans.orientation = math::QuaternionF::FromTo(local, dir);
		SetTransformDirty();
	}

	//! Set a directon of the transformable
	/**
	local_dir and local_up must be orthogonal.
	\param dir The new direction of the local dir axis
	\param up The new direction of the local up axis
	\param local_dir The local direction vector
	\param local_up The local up vector
	*/
	void SetDirectionUp(const math::Vector3F& dir,
		const math::Vector3F& up = math::Vector3F::UNIT_Y,
		const math::Vector3F& local_dir = math::Vector3F::UNIT_Z,
		const math::Vector3F& local_up = math::Vector3F::UNIT_Y)
	{
		m_RelativeTrans.orientation = math::QuaternionF::FromTo(
			local_dir, local_up,
			dir, up);
		SetTransformDirty();
	}

	void LookAt(const math::Vector3F& pos,
		const math::Vector3F& up = math::Vector3F::UNIT_Y,
		const math::Vector3F& local_dir = math::Vector3F::UNIT_Z,
		const math::Vector3F& local_up = math::Vector3F::UNIT_Y)
	{
		SetDirectionUp(pos - GetPosition(), up, local_dir, local_up);
	}

	const math::Transformation& GetTransform()
	{
		return m_RelativeTrans;
	}

	//! transform a relative position to another coordinate system
	/**
	\param point The relative position
	\param target The target coordinate system, use NULL to get absolute coordinates
	\return The transformed position
	*/
	math::Vector3F FromRelativePos(const math::Vector3F& point, Node* target = nullptr)
	{
		math::Vector3F out = GetAbsoluteTransform().TransformPoint(point);
		if(target)
			out = target->GetAbsoluteTransform().TransformInvPoint(out);

		return out;
	}

	//! Transforms a position to relative coordinates
	/**
	\param point The position to transform
	\param source The source coordinate system, use NULL for absolute coordinates
	\return The positon in relative coordinates
	*/
	math::Vector3F ToRelativePos(const math::Vector3F& point, Node* source = nullptr)
	{
		math::Vector3F out;
		if(source)
			out = source->GetAbsoluteTransform().TransformPoint(point);
		else
			out = point;

		return GetAbsoluteTransform().TransformInvPoint(out);
	}

	//! Transforms a relative direction to another coordinate system
	/**
	\param Dir The direction to transform
	\param target The target coordinate system, use NULL for absolute coordinates
	\return The transformed direction
	*/
	math::Vector3F FromRelativeDir(const math::Vector3F& Dir, Node* target = nullptr)
	{
		math::Vector3F out = GetAbsoluteTransform().TransformDir(Dir);
		if(target) {
			out = target->GetAbsoluteTransform().TransformInvDir(out);
		}

		return out;
	}

	//! Transforms a direction to relative coordinates
	/**
	\param dir The direction to transform
	\param source The source coordinate system, use NULL for absolute coordinates
	\return The direction in relative coordinates
	*/
	math::Vector3F ToRelativeDir(const math::Vector3F& dir, Node* source = nullptr)
	{
		math::Vector3F out;
		if(source)
			out = source->GetAbsoluteTransform().TransformDir(dir);
		else
			out = dir;

		return GetAbsoluteTransform().TransformInvDir(out);
	}

	////////////////////////////////////////////////////////////////////////////////

	LUX_API bool IsVisible() const;
	LUX_API bool IsTrulyVisible() const;
	LUX_API void SetVisible(bool visible);
	LUX_API void SwitchVisible();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Node> AddChild(Node* child);
	LUX_API StrongRef<Node> AddChild();
	LUX_API bool HasChildren() const;
	LUX_API void RemoveChild(Node* child);
	LUX_API void RemoveAllChildren();

	//! Delete this node from the scene
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use Scene::AddToDeletionQueue or Node::MarkForDelete
	*/
	void Remove()
	{
		Node* parent = GetParent();
		if(parent)
			parent->RemoveChild(this);
	}
	LUX_API core::Range<ChildIterator> Children();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API void SetParent(Node* newParent);
	LUX_API Node* GetParent() const;
	bool IsRoot() const { return GetParent() == nullptr; }
	LUX_API Scene* GetScene() const;
	LUX_API Node* GetRoot();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Collider> SetCollider(Collider* collider);
	LUX_API StrongRef<Collider> GetCollider() const;

	////////////////////////////////////////////////////////////////////////////////

	//! Retrieve the bounding box of the node.
	LUX_API const math::AABBoxF& GetBoundingBox() const;
	//! Set the bounding box to any value.
	/**
	The bounding box here does not change until Recalculate bounding box.
	is explicit called.
	*/
	LUX_API void SetBoundingBox(const math::AABBoxF& box);
	//! Set the bounding box to the bounding box of all renderables.
	/**
	If new components are added/removed from the node, the bounding
	box is updated automatically.
	*/
	LUX_API void RecalculateBoundingBox();

	////////////////////////////////////////////////////////////////////////////////

	//! Create clone of the current node
	/**
	Clones all children and components.
	But is not attached to any parent.
	*/
	LUX_API StrongRef<Node> Clone() const;
	LUX_API core::Name GetReferableType() const override;

	////////////////////////////////////////////////////////////////////////////////

	bool IsShadowCasting() const { return m_CastShadow; }
	void SetShadowCasting(bool cast) { m_CastShadow = cast; }

	LUX_API void Register(bool doRegister);

protected:
	LUX_API Node(const Node& other);

private:
	void OnAttach();
	void OnDetach();

	void OnAttach(Component* c);
	void OnDetach(Component* c);

	void OnUpdateAnimatedState(Component* c, bool newState);

	void ConditionalUpdateAbsTransform();
	void UpdateAbsTransform();

	bool IsTransformDirty() const { return m_IsTransformDirty; }
	LUX_API void SetTransformDirty();
	void ClearTransformDirty() { m_IsTransformDirty = false; }

	void UpdateTrulyVisible()
	{
		UpdateTrulyVisible(GetParent() ? GetParent()->IsVisible() : true);
	}
	LUX_API void UpdateTrulyVisible(bool parentVisible);
	StrongRef<core::Referable> CloneImpl() const;

private:
	Node* m_Parent; //!< Pointer to the parent of this node
	Node* m_Sibling; //!< Pointer to the sibling(i.e. the next child of the parent) of this node
	Node* m_Child; //!< Pointer to the first child of this node

	u32 m_Tags;

	SceneNodeComponentList m_Components;

	Scene* m_Scene;

	math::Transformation m_AbsoluteTrans;
	math::Transformation m_RelativeTrans;

	StrongRef<Collider> m_Collider;

	math::AABBoxF m_BoundingBox;

	// TODO: Merge all this into a single flag
	bool m_IsVisible;
	bool m_IsTrulyVisible;
	//! Is the current bounding box set by the user.
	bool m_HasUserBoundingBox;
	bool m_CastShadow;
	bool m_InheritTranslation;
	bool m_InheritRotation;
	bool m_InheritScale;
	bool m_IsTransformDirty;
};

} // namespace scene
} // namespace lux

#endif
