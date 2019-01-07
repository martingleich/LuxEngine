#ifndef INCLUDED_LUX_SCENERENDERER_IMPL_H
#define INCLUDED_LUX_SCENERENDERER_IMPL_H
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
class Fog;
class Camera;
class Mesh;

class SceneRendererSimpleForward : public SceneRenderer
{
public:
	SceneRendererSimpleForward(const scene::SceneRendererInitData& data);
	SceneRendererSimpleForward(const SceneRendererSimpleForward&) = delete;
	~SceneRendererSimpleForward();

	void DrawScene(bool beginScene, bool endScene);
	core::VariableAccess Attribute(core::StringView str)
	{
		return m_Attributes[str];
	}
	core::AttributeList AttributeList() const
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

	void EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token);
	void DisableOverwrite(video::PipelineOverwriteToken& token);

	void AddRenderEntry(Node* n, Renderable* r);
	void AddCameraEntry(AbstractCamera* cam);

	void AddLight(Light* l);
	void AddFog(Fog* l);
	void AddAmbient(GlobalAmbientLight* l);

	void DrawScene();
	bool IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum);

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

	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////

	core::Array<RenderEntry> m_SkyBoxList;
	core::Array<RenderEntry> m_SolidNodeList;
	core::Array<DistanceRenderEntry> m_TransparentNodeList;

	core::Array<AbstractCamera*> m_Cameras;
	core::Array<Light*> m_Lights;
	core::Array<Fog*> m_Fogs;
	video::ColorF m_AmbientLight;

	// The current root
	Node* m_CollectedRoot;

	// Information about the current camera
	WeakRef<Node> m_ActiveCameraNode;
	WeakRef<AbstractCamera> m_ActiveCamera;
	math::Vector3F m_AbsoluteCamPos;

	/////////////////////////////////////////////////////////////////////////
	// Settings and parameters
	/////////////////////////////////////////////////////////////////////////
	bool m_Culling; // Read from m_Attributes
	core::AttributeList m_Attributes;
	core::AttributeList m_RendererAttributes;
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