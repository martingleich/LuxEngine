#ifndef INCLUDED_SCENE_NODE_H
#define INCLUDED_SCENE_NODE_H
#include "core/Referable.h"
#include "core/lxName.h"
#include "core/lxArray.h"

#include "scene/Transformable.h"
#include "scene/Component.h"

namespace lux
{
namespace scene
{
class RenderableVisitor;
class Renderable;
class Component;
class Collider;
class Query;
class QueryCallback;
class SceneManager;

enum EDebugData : u32
{
	EDD_NONE = 0,
	EDD_SUB_BBOX = 1,        // Alle Unterboundingboxen zeichen(z.B. Submeshes)
	EDD_MAIN_BBOX = 2,        // Nur die Hautpboundingbox zeichnen
	EDD_ALL_BBOX = EDD_SUB_BBOX | EDD_MAIN_BBOX,        // Sowohl Haupt- als auch Unterboundingbox zeichnen
};

class Node : public Referable, public scene::Transformable
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

	class ChildIterator : core::BaseIterator<core::BidirectionalIteratorTag, Node*>
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
	LUX_API Node(SceneManager* creator, bool isRoot = false);
	LUX_API virtual ~Node();

	LUX_API virtual void VisitRenderables(RenderableVisitor* visitor, bool noDebug);
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
	LUX_API virtual ComponentIterator GetComponentsFirst();
	LUX_API virtual ComponentIterator GetComponentsEnd();
	LUX_API virtual ConstComponentIterator GetComponentsFirst() const;
	LUX_API virtual ConstComponentIterator GetComponentsEnd() const;

	template <typename T>
	T* GetComponent(T* cur = nullptr)
	{
		for(auto it = GetComponentsFirst(); it != GetComponentsEnd(); ++it) {
			T* p = dynamic_cast<T*>(*it);
			if(p && p != cur)
				return p;
		}

		return nullptr;
	}

	template <typename T>
	const T* GetComponent(const T* cur = nullptr) const
	{
		for(auto it = GetComponentsFirst(); it != GetComponentsEnd(); ++it) {
			const T* p = dynamic_cast<const T*>(*it);
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

	LUX_API virtual void SetRelativeTransform(const math::Transformation& t);
	LUX_API virtual const math::Transformation& GetAbsoluteTransform() const;
	LUX_API virtual const math::vector3f& GetAbsolutePosition() const
	{
		return GetAbsoluteTransform().translation;
	}

	LUX_API virtual const math::Transformation& GetRelativeTransform() const;

	//! transform a relative position to another coordinate system
	/**
	\param point The relative position
	\param target The target coordinate system, use NULL to get absolute coordinates
	\return The transformed position
	*/
	math::vector3f FromRelativePos(const math::vector3f& point, const Node* target = nullptr) const
	{
		math::vector3f out = GetAbsoluteTransform().TransformPoint(point);
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
	math::vector3f ToRelativePos(const math::vector3f& point, const Node* source = nullptr) const
	{
		math::vector3f out;
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
	math::vector3f FromRelativeDir(const math::vector3f& Dir, const Node* target = nullptr) const
	{
		math::vector3f out = GetAbsoluteTransform().TransformDir(Dir);
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
	math::vector3f ToRelativeDir(const math::vector3f& dir, const Node* source = nullptr) const
	{
		math::vector3f out;
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
	To delete a node while doing this use SceneManager::AddToDeletionQueue or Node::MarkForDelete
	*/
	LUX_API virtual void Remove();
	LUX_API ChildIterator GetChildrenFirst();
	LUX_API ChildIterator GetChildrenEnd();
	LUX_API ConstChildIterator GetChildrenFirst() const;
	LUX_API ConstChildIterator GetChildrenEnd() const;

	////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual void SetParent(Node* newParent);
	LUX_API Node* GetParent() const;
	LUX_API SceneManager* GetSceneManager() const;
	LUX_API Node* GetRoot();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API void SetDebugData(EDebugData debugData, bool state);
	LUX_API bool GetDebugData(EDebugData debugData);

	////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Collider> SetCollider(Collider* collider);
	LUX_API StrongRef<Collider> GetCollider() const;

	LUX_API bool ExecuteQuery(Query* query, QueryCallback* queryCallback);

	////////////////////////////////////////////////////////////////////////////////

	LUX_API const math::aabbox3df& GetBoundingBox() const;
	LUX_API void SetBoundingBox(const math::aabbox3df& box);
	LUX_API void RecalculateBoundingBox();

	////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Referable> Clone() const;
	LUX_API virtual core::Name GetReferableType() const;

	////////////////////////////////////////////////////////////////////////////////

protected:
	LUX_API Node(const Node& other);

private:
	void OnAttach();
	void OnDettach();

	void OnAddComponent(Component* c);
	void OnRemoveComponent(Component* c);

	bool UpdateAbsTransform() const;

	void SetDirty() const;

private:
	Node* m_Parent; //!< Pointer to the parent of this node
	Node* m_Sibling; //!< Pointer to the sibling(i.e. the next child of the parent) of this node
	Node* m_Child; //!< Pointer to the first child of this node

	u32 m_Tags;

	u32 m_DebugFlags;

	u32 m_AnimatedCount;
	SceneNodeComponentList m_Components;

	SceneManager* m_SceneManager;

	mutable math::Transformation m_AbsoluteTrans;

	StrongRef<Collider> m_Collider;

	math::aabbox3df m_BoundingBox;

	bool m_IsVisible;
	bool m_HasUserBoundingBox;
	bool m_IsRoot;
};

} // namespace scene
} // namespace lux

#endif
