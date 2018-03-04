#ifndef INCLUDED_SCENERENDERER_IMPL_H
#define INCLUDED_SCENERENDERER_IMPL_H
#include "scene/SceneRenderer.h"

#include "core/lxOrderedMap.h"

#include "video/Renderer.h"

#include "scene/Scene.h"
#include "scene/Node.h"
#include "scene/components/Camera.h"

#include "scene/StencilShadowRenderer.h"

namespace lux
{
namespace scene
{
class GlobalAmbientLight;
class SceneDataCollector : public SceneObserver
{
public:
	void OnAttach(Node* node);
	void OnDetach(Node* node);

	void OnAttach(Component* comp);
	void OnDetach(Component* comp);

	void RegisterCamera(AbstractCamera* camera);
	void UnregisterCamera(AbstractCamera* camera);

	void RegisterLight(Light* light);
	void UnregisterLight(Light* light);

	void RegisterFog(GlobalFog* fog);
	void UnregisterFog(GlobalFog* fog);

	core::Array<AbstractCamera*> cameraList; //!< The cameras of the graph
	core::Array<Light*> lightList; //!< The lights of the graph
	core::Array<GlobalFog*> fogList; //!< The fogs of the graph
	GlobalAmbientLight* ambientLight = nullptr;
};

class SceneRendererSimpleForward : public SceneRenderer
{
public:
	SceneRendererSimpleForward(const core::ModuleInitData& data);
	SceneRendererSimpleForward(const SceneRendererSimpleForward&) = delete;
	~SceneRendererSimpleForward();

	void DrawScene(bool beginScene, bool endScene);
	core::VariableAccess Attribute(const core::String& str)
	{
		return m_Attributes[str];
	}
	const core::Attributes& Attributes() const
	{
		return m_Attributes;
	}

	void SetPassSettings(ERenderPass pass, const SceneRendererPassSettings& settings)
	{
		m_PassSettings[pass] = settings;
	}
	const SceneRendererPassSettings& GetPassSettings(ERenderPass pass)
	{
		return m_PassSettings[pass];
	}

	virtual void EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token);
	virtual void DisableOverwrite(video::PipelineOverwriteToken& token);

	virtual void AddRenderEntry(Node* n, Renderable* r);

	virtual void DrawScene();
	virtual bool IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum);

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

		RenderEntry(Node* n, Renderable* r, bool c = false) :
			node(n),
			renderable(r),
			isCulled(c)
		{
		}

		bool IsCulled() const
		{
			return isCulled;
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
	StrongRef<Scene> m_Scene;

	StrongRef<SceneDataCollector> m_SceneData;

	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////

	core::Array<RenderEntry> m_SkyBoxList;
	core::Array<RenderEntry> m_SolidNodeList;
	core::Array<DistanceRenderEntry> m_TransparentNodeList;

	Node* m_CollectedRoot;

	// Information about the current camera
	WeakRef<Node> m_ActiveCameraNode;
	WeakRef<AbstractCamera> m_ActiveCamera;
	math::Vector3F m_AbsoluteCamPos;

	/////////////////////////////////////////////////////////////////////////
	// Settings and parameters
	/////////////////////////////////////////////////////////////////////////
	bool m_Culling; // Read from m_Attributes
	core::Attributes m_Attributes;
	core::HashMap<ERenderPass, SceneRendererPassSettings> m_PassSettings;

	bool m_SettingsActive;

	/////////////////////////////////////////////////////////////////////////
	// References to other classes
	/////////////////////////////////////////////////////////////////////////

	video::Renderer* m_Renderer;
	StencilShadowRenderer m_StencilShadowRenderer;
};

} // namespace scene
} // namespace lux

#endif