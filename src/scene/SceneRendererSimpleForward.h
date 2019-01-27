#ifndef INCLUDED_LUX_SCENERENDERER_IMPL_H
#define INCLUDED_LUX_SCENERENDERER_IMPL_H
#include "scene/SceneRenderer.h"

#include "core/lxOrderedMap.h"

#include "video/Renderer.h"

#include "scene/Scene.h"
#include "scene/Node.h"

#include "scene/components/Camera.h"
#include "scene/components/Fog.h"
#include "scene/components/Light.h"

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

	void AddRenderEntry(Node* n, Component* r);

	void DrawScene();
	bool IsCulled(Node* node, Component* r, const math::ViewFrustum& frustum);

private:
	struct RenderEntry
	{
	public:
		Node* node;
		Component* renderable;

		RenderEntry() :
			node(nullptr),
			renderable(nullptr)
		{
		}

		RenderEntry(Node* n, Component* r) :
			node(n),
			renderable(r)
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

		DistanceRenderEntry(Node* n, Component* r) :
			RenderEntry(n, r)
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

	void ClearLightData(video::Renderer* renderer);
	void AddLightData(video::Renderer* renderer, ClassicalLightDescription* desc);

private:
	StrongRef<Scene> m_Scene;

	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////

	core::Array<RenderEntry> m_SkyBoxList;
	core::Array<RenderEntry> m_SolidNodeList;
	core::Array<RenderEntry> m_ShadowCasters;
	core::Array<DistanceRenderEntry> m_TransparentNodeList;

	core::Array<AbstractCamera*> m_Cameras;
	core::Array<Light*> m_Lights;
	core::Array<Fog*> m_Fogs;

	// The current root
	Node* m_CollectedRoot;

	// Information about the current camera
	WeakRef<Node> m_ActiveCameraNode;
	WeakRef<AbstractCamera> m_ActiveCamera;
	math::Vector3F m_AbsoluteCamPos;
	math::ViewFrustum m_ActiveFrustum;

	int m_CurLightId;

	/////////////////////////////////////////////////////////////////////////
	// Settings and parameters
	/////////////////////////////////////////////////////////////////////////
	bool m_Culling; // Cached: Read from m_Attributes
	int m_MaxLightsPerDraw = 4;
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