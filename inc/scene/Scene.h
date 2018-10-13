#ifndef INCLUDED_LUX_SCENE_H
#define INCLUDED_LUX_SCENE_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/lxOrderedSet.h"
#include "core/lxName.h"

#include "scene/Renderable.h"

namespace lux
{
namespace scene
{
class Component;
class Node;

class SceneObserver : public virtual ReferenceCounted
{
public:
	// Only called for parent node
	virtual void OnAttach(Node* child) = 0;
	// Only called for parent node
	virtual void OnDetach(Node* child) = 0;

	// Only if component is attached to a node.
	// Not if a node with components is attached to a node in the scene.
	virtual void OnAttach(Component* child) = 0;
	// Only if component is detached to a node.
	// Not if a node with components is detached to a node in the scene.
	virtual void OnDetach(Component* child) = 0;
};

class ComponentVisitor
{
public:
	virtual void Visit(Node* n, Component* c) = 0;
	void AbortChildren() { m_AbortChildren = true; }
	void ResumeChildren() { m_AbortChildren = false; }
	bool ShouldAbortChildren() const { return m_AbortChildren; }
private:
	bool m_AbortChildren = false;
};

class Scene : public ReferenceCounted
{
public:
	LUX_API Scene();
	Scene(const Scene&) = delete;
	LUX_API ~Scene();

	LUX_API void RegisterObserver(SceneObserver* observer);
	LUX_API void UnregisterObserver(SceneObserver* observer);

	LUX_API void Clear();
	LUX_API void AddToDeletionQueue(Node* node);
	LUX_API void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API Node* GetRoot() const;
	LUX_API bool IsEmpty() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void RegisterAnimated(Node* node);
	LUX_API void UnregisterAnimated(Node* node);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void AnimateAll(float secsPassed);
	LUX_API void VisitRenderables(
		RenderableVisitor* visitor,
		ERenderableTags tags = ERenderableTags::None, Node* root = nullptr);

	//! Visit all components.
	/*
	Visits the components in top-down order.
	\param visitor The visitor.
	\param root The root of the tree to visit, if null the scene-root is used.
	*/
	LUX_API void VisitComponents(
		ComponentVisitor* visitor,
		Node* root = nullptr);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void OnAttach(Node* node);
	LUX_API void OnDetach(Node* node);

	LUX_API void OnAttach(Component* comp);
	LUX_API void OnDetach(Component* comp);

private:
	StrongRef<Node> m_Root; //!< The root of the scenegraph

	core::Array<StrongRef<SceneObserver>> m_Observers;
	core::Array<StrongRef<Node>> m_DeletionQueue; //!< Nodes to delete on next deletion run

	core::OrderedSet<Node*> m_AnimatedNodes; //!< The animated nodes of the graph
};

} // namespace scene
} // namespace lux

#endif
