#ifndef INCLUDED_LUX_SCENE_H
#define INCLUDED_LUX_SCENE_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/lxHashSet.h"

#include "scene/Node.h"

namespace lux
{
namespace scene
{

class ComponentVisitor
{
public:
	virtual ~ComponentVisitor() {}
	virtual void Visit(Component* c) = 0;
	void AbortChildren() { m_AbortChildren = true; }
	void ResumeChildren() { m_AbortChildren = false; }
	bool ShouldAbortChildren() const { return m_AbortChildren; }

private:
	bool m_AbortChildren = false;
};

class SceneDebugSettings
{
public:
	bool renderWireframe = false;
};

class InternalRenderData;
class Scene : public ReferenceCounted, core::Uncopyable
{
public:
	LUX_API Scene();
	LUX_API ~Scene();

	LUX_API void AddToDeletionQueue(Node* node);
	LUX_API void AddToDeletionQueue(Component* node);
	LUX_API void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);

	////////////////////////////////////////////////////////////////////////////////////

	inline Node* GetRoot() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void RegisterForTick(Component* c, bool doRegister);
	LUX_API void RegisterLight(Component* c, bool doRegister);
	LUX_API void RegisterFog(Component* c, bool doRegister);
	LUX_API void RegisterRenderController(SceneRenderPassController* c, bool doRegister);

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

	/*
	Renders the scene.
		Changes the state of the video renderer.
		The last rendered pass(normally the backbuffer) isn't ended.
		This makes it possible to keep rendering data(for example the gui).
		This means VideoRenderer::EndScene() must be called.
		This means the last set rendertarget is still active.
	*/
	LUX_API void DrawScene();

	////////////////////////////////////////////////////////////////////////////////////
	// TODO: Improve configuration of scenes.
	inline core::AttributeList AttributeList() const;

	inline void SetDebugSettings(const SceneDebugSettings& settings);
	inline const SceneDebugSettings& GetDebugSettings();

private:
	StrongRef<Node> m_Root; //!< The root of the scenegraph

	core::Array<StrongRef<Node>> m_NodeDeletionQueue; //!< Nodes to delete on next deletion run
	core::Array<StrongRef<Component>> m_CompDeletionQueue; //!< Nodes to delete on next deletion run
	
	// TODO: Use better datastructure for this.
	core::HashSet<Component*> m_AnimatedComps; //!< The animated nodes of the graph

	core::AttributeList m_Attributes;

	SceneDebugSettings m_DebugSettings;

	std::unique_ptr<InternalRenderData> renderData;
};

inline Node* Scene::GetRoot() const { return m_Root; }
inline core::AttributeList Scene::AttributeList() const { return m_Attributes; }
inline void Scene::SetDebugSettings(const SceneDebugSettings& settings) { m_DebugSettings = settings; }
inline const SceneDebugSettings& Scene::GetDebugSettings() { return m_DebugSettings; }

} // namespace scene
} // namespace lux

#endif
