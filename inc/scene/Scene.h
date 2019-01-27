#ifndef INCLUDED_LUX_SCENE_H
#define INCLUDED_LUX_SCENE_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/lxHashSet.h"
#include "core/lxName.h"

#include "scene/Renderable.h"

namespace lux
{
namespace scene
{
class Component;
class Node;

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

	LUX_API void AddToDeletionQueue(Node* node);
	LUX_API void AddToDeletionQueue(Component* node);
	LUX_API void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API Node* GetRoot() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void RegisterForTick(Component* c, bool doRegister);
	LUX_API void RegisterLight(Component* c, bool doRegister);
	LUX_API void RegisterFog(Component* c, bool doRegister);
	LUX_API void RegisterCamera(Component* c, bool doRegister);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void AnimateAll(float secsPassed);

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

	core::Range<core::HashSet<Component*>::Iterator> GetLights() { return core::MakeRange(m_LightComps); }
	core::Range<core::HashSet<Component*>::Iterator> GetFogs() { return core::MakeRange(m_FogComps); }
	core::Range<core::HashSet<Component*>::Iterator> GetCameras() { return core::MakeRange(m_CamComps); }

private:
	StrongRef<Node> m_Root; //!< The root of the scenegraph

	core::Array<StrongRef<Node>> m_NodeDeletionQueue; //!< Nodes to delete on next deletion run
	core::Array<StrongRef<Component>> m_CompDeletionQueue; //!< Nodes to delete on next deletion run

	// TODO: Use better datastructure for this.
	core::HashSet<Component*> m_AnimatedComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_LightComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_FogComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_CamComps; //!< The animated nodes of the graph
};

} // namespace scene
} // namespace lux

#endif
