#ifndef INCLUDED_SCENERENDERER_IMPL_H
#define INCLUDED_SCENERENDERER_IMPL_H
#include "scene/SceneRenderer.h"

#include "core/lxOrderedMap.h"
#include "video/Renderer.h"

#include "scene/components/Camera.h"
#include "scene/Node.h"

#include "scene/StencilShadowRenderer.h"

namespace lux
{
namespace scene
{

class SceneRendererImpl : public SceneRenderer
{
public:
	SceneRendererImpl();
	~SceneRendererImpl();

	////////////////////////////////////////////////////////////////////////////////////

	void DrawScene(Scene* scene, bool beginScene, bool endScene);

	////////////////////////////////////////////////////////////////////////////////////

	void PushPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over);
	void PopPipelineOverwrite(ERenderPass pass);

	////////////////////////////////////////////////////////////////////////////////////

	core::VariableAccess Attribute(const String& str)
	{
		return m_Attributes[str];
	}

	const core::Attributes& Attributes() const
	{
		return m_Attributes;
	}

private:
	void EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token);
	void DisableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token);

	class RenderableCollector;
	void CollectRenderables(Node* root);
	void CollectRenderablesRec(Node* node, RenderableCollector* collector, bool noDebug);
	void AddRenderEntry(Node* n, Renderable* r);

	void DrawScene();

	struct RenderEntry;
	bool IsCulled(const RenderEntry& e);
	bool IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum);

	void AddDriverLight(Node* n, Light* l);

private:
	struct RenderEntry
	{
	public:
		Node* node;
		Renderable* renderable;
		bool isCulled;

		RenderEntry() :
			node(nullptr),
			renderable(nullptr),
			isCulled(false)
		{
		}

		RenderEntry(Node* n, Renderable* r, bool c=false) :
			node(n),
			renderable(r),
			isCulled(c)
		{
		}
	};

	struct DistanceRenderEntry : public RenderEntry
	{
		float distance;
		math::Vector3F pos;

		DistanceRenderEntry()
		{
		}

		DistanceRenderEntry(Node* n, Renderable* r, bool c) :
			RenderEntry(n, r, c)
		{
			pos = node->GetAbsoluteTransform().TransformPoint(r->GetBoundingBox().GetCenter());
		}

		void UpdateDistance(const math::Vector3F& camera)
		{
			distance = pos.GetDistanceToSq(camera);
		}

		bool operator<(const DistanceRenderEntry& other) const
		{
			// The farthest element must be first in list
			return distance > other.distance;
		}
	};

private:
	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////
	Scene* m_Scene;

	core::Array<RenderEntry> m_SkyBoxList;
	core::Array<RenderEntry> m_SolidNodeList;
	core::Array<DistanceRenderEntry> m_TransparentNodeList;

	Node* m_CollectedRoot;

	// Information about the current camera
	WeakRef<Node> m_ActiveCameraNode;
	WeakRef<Camera> m_ActiveCamera;
	math::Vector3F m_AbsoluteCamPos;

	/////////////////////////////////////////////////////////////////////////
	// Settings and parameters
	/////////////////////////////////////////////////////////////////////////
	core::Attributes m_Attributes;
	bool m_Culling; // Read from m_Attributes

	core::OrderedMap<ERenderPass, core::Array<video::PipelineOverwrite>> m_Overwrites; //!< User-set pipeline overwrites

	/////////////////////////////////////////////////////////////////////////
	// References to other classes
	/////////////////////////////////////////////////////////////////////////

	video::Renderer* m_Renderer;
	StencilShadowRenderer m_StencilShadowRenderer;
};

} // namespace scene
} // namespace lux

#endif