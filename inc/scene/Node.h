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

class Node : public Referable
{
private:
	class ComponentEntry
	{
	public:
		ComponentEntry() {}
		ComponentEntry(Component* c) : comp(c),
			markForDelete(false)
		{
		}

		StrongRef<Component> comp;
		bool markForDelete;
	};

	typedef core::Array<ComponentEntry> SceneNodeComponentList;

public:
	class ComponentIterator : core::BaseIterator<core::BidirectionalIteratorTag, Component*>
	{
	private:
		friend class ConstComponentIterator;
		friend class Node;

		explicit ComponentIterator(typename SceneNodeComponentList::Iterator begin) : m_Current(begin)
		{
		}

	public:
		ComponentIterator()
		{
		}

		ComponentIterator& operator++()
		{
			++m_Current; return *this;
		}

		ComponentIterator  operator++(int)
		{
			ComponentIterator temp = *this; ++m_Current; return temp;
		}

		ComponentIterator& operator+=(unsigned int num)
		{
			while(num--) ++(*this);

			return *this;
		}

		ComponentIterator  operator+ (unsigned int num) const
		{
			ComponentIterator temp = *this; return temp += num;
		}

		bool operator==(const ComponentIterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ComponentIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		Component* operator*()
		{
			return Pointer();
		}
		Component* operator->()
		{
			return Pointer();
		}

		Component* Pointer()
		{
			return m_Current->comp;
		}

	private:
		SceneNodeComponentList::Iterator m_Current;
	};

	class ConstComponentIterator : core::BaseIterator<core::BidirectionalIteratorTag, Component*>
	{
	private:
		friend class Node;

		explicit ConstComponentIterator(typename SceneNodeComponentList::ConstIterator begin) : m_Current(begin)
		{
		}

	public:
		ConstComponentIterator()
		{
		}

		ConstComponentIterator& operator++()
		{
			++m_Current; return *this;
		}

		ConstComponentIterator  operator++(int)
		{
			ConstComponentIterator temp = *this; ++m_Current; return temp;
		}

		ConstComponentIterator& operator+=(unsigned int num)
		{
			while(num--) ++(*this);

			return *this;
		}

		ConstComponentIterator  operator+ (unsigned int num) const
		{
			ConstComponentIterator temp = *this; return temp += num;
		}

		bool operator==(const ConstComponentIterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ConstComponentIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		bool operator==(const ComponentIterator& other) const
		{
			return m_Current == other.m_Current;
		}

		bool operator!=(const ComponentIterator& other) const
		{
			return m_Current != other.m_Current;
		}
		const Component* operator*()
		{
			return Pointer();
		}
		const Component* operator->()
		{
			return Pointer();
		}

		const Component* Pointer()
		{
			return m_Current->comp;
		}

	private:
		SceneNodeComponentList::ConstIterator m_Current;
	};
	class ConstChildIterator;

	class ChildIterator : public core::BaseIterator<core::BidirectionalIteratorTag, Node*>
	{
	public:
		ChildIterator() : m_Current(nullptr)
		{
		}

		ChildIterator& operator++()
		{
			m_Current = m_Current->m_Sibling; return *this;
		}
		ChildIterator  operator++(int)
		{
			ChildIterator Temp = *this; m_Current = m_Current->m_Sibling; return Temp;
		}

		ChildIterator& operator+=(unsigned int num)
		{
			while(num-- && this->m_Current != 0) ++(*this);

			return *this;
		}

		ChildIterator  operator+ (unsigned int num) const
		{
			ChildIterator temp = *this; return temp += num;
		}

		bool operator==(const ChildIterator&        other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ChildIterator&        other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const ConstChildIterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ConstChildIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		Node* operator*()
		{
			return m_Current;
		}
		Node* operator->()
		{
			return m_Current;
		}

		Node* Pointer()
		{
			return m_Current;
		}

	private:
		explicit ChildIterator(Node* begin) : m_Current(begin)
		{
		}
		friend class Node;
		friend class ConstChildIterator;

	private:
		Node* m_Current;
	};

	class ConstChildIterator : core::BaseIterator<core::BidirectionalIteratorTag, Node*>
	{
	public:
		ConstChildIterator() : m_Current(nullptr)
		{
		}

		ConstChildIterator& operator++()
		{
			m_Current = m_Current->m_Sibling; return *this;
		}
		ConstChildIterator  operator++(int)
		{
			ConstChildIterator Temp = *this; m_Current = m_Current->m_Sibling; return Temp;
		}

		ConstChildIterator& operator+=(unsigned int num)
		{
			while(num-- && this->m_Current != nullptr) ++(*this);

			return *this;
		}

		ConstChildIterator  operator+ (unsigned int num) const
		{
			ConstChildIterator temp = *this; return temp += num;
		}

		bool operator==(const ChildIterator&         other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ChildIterator&         other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const ConstChildIterator& other) const
		{
			return m_Current == other.m_Current;
		}

		bool operator!=(const ConstChildIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		const Node* operator*()
		{
			return m_Current;
		}
		const Node* operator->()
		{
			return m_Current;
		}

		ConstChildIterator& operator=(const ChildIterator& iter)
		{
			m_Current = iter.m_Current; return *this;
		}

		const Node* Pointer() const
		{
			return m_Current;
		}

	private:
		explicit ConstChildIterator(const Node* begin) : m_Current(begin)
		{
		}

		friend class ChildIterator;
		friend class Node;

	private:
		const Node* m_Current;
	};

public:
	LUX_API Node(Scene* scene, bool isRoot = false);
	LUX_API virtual ~Node();

	LUX_API virtual void VisitRenderables(RenderableVisitor* visitor, ERenderableTags tags);
	LUX_API virtual void Animate(float time);

	////////////////////////////////////////////////////////////////////////////////
	//! Marks a scene node componenent for deletion.
	/**
	This method can be called at any time in the program.
	The component will be removed before the next frame.
	\param comp The component to remove.
	*/
	LUX_API void MarkForDelete(Component* comp);
	LUX_API void MarkForDelete();
	LUX_API virtual void CleanDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual StrongRef<Component> AddComponent(Component* component);
	LUX_API virtual core::Range<ComponentIterator> Components();
	LUX_API virtual core::Range<ConstComponentIterator> Components() const;

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
	const T* GetComponent(const T* cur = nullptr) const
	{
		for(auto comp : Components()) {
			const T* p = dynamic_cast<const T*>(comp);
			if(p && p != cur)
				return p;
		}

		return nullptr;
	}

	template <typename T>
	bool HasComponent() const
	{
		return (GetComponent<T>() != nullptr);
	}

	LUX_API virtual bool HasComponents() const;

	//! Removes a component from the scene node
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use Node::MarkForDelete.
	\param comp The component to remove from the node.
	\return Was the component found, and deleted.
	*/
	LUX_API virtual void RemoveComponent(Component* comp);

	//! Removes all components
	LUX_API virtual void RemoveAllComponents();

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

	LUX_API void SetRelativeTransform(const math::Transformation& t);
	LUX_API const math::Transformation& GetAbsoluteTransform() const;
	LUX_API const math::Vector3F& GetAbsolutePosition() const
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
			SetDirty();
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
			SetDirty();
		}
	}

	//! Set a new position
	/**
	\param p The new position
	*/
	void SetPosition(const math::Vector3F& p)
	{
		m_RelativeTrans.translation = p;
		SetDirty();
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
		SetDirty();
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
		SetDirty();
	}

	//! Apply a rotation to the transformable
	/**
	\param o The orientation to apply.
	*/
	void Rotate(const math::QuaternionF& o)
	{
		m_RelativeTrans.orientation *= o;
		SetDirty();
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
	float GetScale() const
	{
		return m_RelativeTrans.scale;
	}

	//! Get the current relative position
	/**
	\return The current relative position
	*/
	const math::Vector3F& GetPosition() const
	{
		return m_RelativeTrans.translation;
	}

	//! Get the current relative orientation
	/**
	\return The current relative orientation
	*/
	const math::QuaternionF& GetOrientation() const
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
		SetDirty();
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
		SetDirty();
	}

	void LookAt(const math::Vector3F& pos,
		const math::Vector3F& up = math::Vector3F::UNIT_Y,
		const math::Vector3F& local_dir = math::Vector3F::UNIT_Z,
		const math::Vector3F& local_up = math::Vector3F::UNIT_Y)
	{
		SetDirectionUp(pos - GetPosition(), up, local_dir, local_up);
	}

	const math::Transformation& GetTransform() const
	{
		return m_RelativeTrans;
	}

	//! transform a relative position to another coordinate system
	/**
	\param point The relative position
	\param target The target coordinate system, use NULL to get absolute coordinates
	\return The transformed position
	*/
	math::Vector3F FromRelativePos(const math::Vector3F& point, const Node* target = nullptr) const
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
	math::Vector3F ToRelativePos(const math::Vector3F& point, const Node* source = nullptr) const
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
	math::Vector3F FromRelativeDir(const math::Vector3F& Dir, const Node* target = nullptr) const
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
	math::Vector3F ToRelativeDir(const math::Vector3F& dir, const Node* source = nullptr) const
	{
		math::Vector3F out;
		if(source)
			out = source->GetAbsoluteTransform().TransformDir(dir);
		else
			out = dir;

		return GetAbsoluteTransform().TransformInvDir(out);
	}

	////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual bool IsVisible() const;
	LUX_API virtual bool IsTrulyVisible() const;
	LUX_API virtual void SetVisible(bool visible);
	LUX_API virtual void SwitchVisible();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual StrongRef<Node> AddChild(Node* child);
	LUX_API virtual StrongRef<Node> AddChild();
	LUX_API virtual bool HasChildren() const;
	LUX_API virtual void RemoveChild(Node* child);
	LUX_API virtual void RemoveAllChildren();

	//! Delete this node from the scene
	/**
	Use this method only when currently not animating or rendering a scene.
	To delete a node while doing this use Scene::AddToDeletionQueue or Node::MarkForDelete
	*/
	LUX_API virtual void Remove();
	LUX_API core::Range<ChildIterator> Children();
	LUX_API core::Range<ConstChildIterator> Children() const;

	////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual void SetParent(Node* newParent);
	LUX_API Node* GetParent() const;
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
	LUX_API virtual core::Name GetReferableType() const;

	////////////////////////////////////////////////////////////////////////////////

	LUX_API bool IsShadowCasting() const;
	LUX_API void SetShadowCasting(bool cast);

protected:
	LUX_API Node(const Node& other);

private:
	void OnAttach();
	void OnDetach();

	void OnAttach(Component* c);
	void OnDetach(Component* c);

	bool UpdateAbsTransform() const;

	bool IsDirty() const
	{
		return m_IsDirty;
	}
	LUX_API void SetDirty() const;
	void ClearDirty() const
	{
		m_IsDirty = false;
	}

	StrongRef<Referable> CloneImpl() const;

private:
	Node* m_Parent; //!< Pointer to the parent of this node
	Node* m_Sibling; //!< Pointer to the sibling(i.e. the next child of the parent) of this node
	Node* m_Child; //!< Pointer to the first child of this node

	u32 m_Tags;

	int m_AnimatedCount;
	SceneNodeComponentList m_Components;

	Scene* m_Scene;

	mutable math::Transformation m_AbsoluteTrans;
	math::Transformation m_RelativeTrans;

	StrongRef<Collider> m_Collider;

	math::AABBoxF m_BoundingBox;

	bool m_IsVisible;
	//! Is the current bounding box set by the user.
	bool m_HasUserBoundingBox;
	bool m_IsRoot;
	bool m_CastShadow;
	mutable bool m_IsDirty;
};

} // namespace scene
} // namespace lux

#endif
