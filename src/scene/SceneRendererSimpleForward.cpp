#include "scene/SceneRendererSimpleForward.h"
#include "core/Logger.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "video/mesh/VideoMesh.h"

#include "core/lxAlgorithm.h"

#include "scene/components/Fog.h"
#include "scene/components/Light.h"
#include "scene/components/SceneMesh.h"

#include "core/ModuleFactory.h"

LUX_REGISTER_MODULE("SceneRenderer", "SimpleForward", lux::scene::SceneRendererSimpleForward)

namespace lux
{
namespace scene
{

SceneRendererSimpleForward::SceneRendererSimpleForward(const core::ModuleInitData& data) :
	m_CollectedRoot(nullptr),
	m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF)
{
	LUX_UNUSED(data);

	m_Renderer = video::VideoDriver::Instance()->GetRenderer();
	m_Scene = dynamic_cast<const scene::SceneRendererInitData&>(data).scene;

	m_Attributes.AddAttribute("drawStencilShadows", false);
	m_Attributes.AddAttribute("maxShadowCasters", 1);
	m_Attributes.AddAttribute("culling", true);
}

SceneRendererSimpleForward::~SceneRendererSimpleForward()
{
}

namespace
{
class RenderableCollector : public RenderableVisitor
{
public:
	RenderableCollector(SceneRendererSimpleForward* _sr) :
		sr(_sr)
	{
	}

	void Visit(Node* node, Renderable* r)
	{
		sr->AddRenderEntry(node, r);
	}

	SceneRendererSimpleForward* sr;
};

class CameraCollector : public ComponentVisitor
{
public:
	CameraCollector(SceneRendererSimpleForward* _sr) :
		sr(_sr)
	{
	}

	void Visit(Node* node, Component* r)
	{
		if(!node->IsVisible()) {
			AbortChildren();
			return;
		}
		if(auto cam = dynamic_cast<AbstractCamera*>(r))
			sr->AddCameraEntry(cam);
	}

	SceneRendererSimpleForward* sr;
};

class LightFogCollector : public ComponentVisitor
{
public:
	LightFogCollector(SceneRendererSimpleForward* _sr) :
		sr(_sr)
	{
	}

	void Visit(Node* node, Component* comp)
	{
		if(!node->IsVisible()) {
			AbortChildren();
			return;
		}
		auto fog = dynamic_cast<GlobalFog*>(comp);
		if(fog)
			sr->AddFog(fog);
		auto light = dynamic_cast<Light*>(comp);
		if(light)
			sr->AddLight(light);
		auto ambient = dynamic_cast<GlobalAmbientLight*>(comp);
		if(ambient)
			sr->AddAmbient(ambient);
	}

	SceneRendererSimpleForward* sr;
};

struct CameraSortT
{
	bool Smaller(const AbstractCamera* a, const AbstractCamera* b) const
	{
		return a->GetRenderPriority() > b->GetRenderPriority();
	}
};
}

void SceneRendererSimpleForward::DrawScene(bool beginScene, bool endScene)
{
	video::RenderStatistics::GroupScope grpScope("scene");

	CameraCollector camCollector(this);
	m_Cameras.Clear();
	m_Scene->VisitComponents(&camCollector);

	// Collect "real" camera nodes
	auto newEnd = core::RemoveIf(m_Cameras,
		[](const AbstractCamera* c) { return !c->GetParent()->IsTrulyVisible(); }
	);
	m_Cameras.Resize(core::IteratorDistance(m_Cameras.First(), newEnd));
	m_Cameras.Sort(CameraSortT());

	if(m_Cameras.Size() == 0) {
		if(!m_Scene->IsEmpty())
			log::Warning("No camera in scenegraph.");
		if(beginScene == true) {
			m_Renderer->Clear(true, true, true);
			m_Renderer->BeginScene();
		} if(endScene)
			m_Renderer->EndScene();
		return;
	}

	if(!beginScene) {
		if(m_Renderer->GetRenderTarget() != m_Cameras[0]->GetRenderTarget())
			throw core::GenericRuntimeException("Already started scene uses diffrent rendertarget than first camera.");
	}

	for(auto it = m_Cameras.First(); it != m_Cameras.End(); ++it) {
		m_ActiveCamera = *it;
		m_ActiveCameraNode = m_ActiveCamera->GetParent();
		m_AbsoluteCamPos = m_ActiveCameraNode->GetAbsolutePosition();

		if(it != m_Cameras.First() || beginScene) {
			// Start a new scene
			m_Renderer->SetRenderTarget(m_ActiveCamera->GetRenderTarget());
			bool clearColor = true;
			bool clearZ = true;
			bool clearStencil = true;
			if(!m_SkyBoxList.IsEmpty()) {
				clearZ = true;
				clearColor = false;
			}
			m_Renderer->Clear(clearColor, clearZ, clearStencil);
			m_Renderer->BeginScene();
		}

		m_ActiveCamera->PreRender(m_Renderer);

		m_ActiveCamera->Render(m_Renderer);

		RenderableCollector collector(this);
		m_Scene->VisitRenderables(&collector, ERenderableTags::None);
		// Collect lights, fogs, and other non renderable global things.
		LightFogCollector lfCollector(this);
		m_Scene->VisitComponents(&lfCollector);

		DrawScene();
		m_ActiveCamera->PostRender(m_Renderer);

		if(it != m_Cameras.Last() || endScene)
			m_Renderer->EndScene();

		m_SkyBoxList.Clear();
		m_SolidNodeList.Clear();
		m_TransparentNodeList.Clear();
		m_Lights.Clear();
		m_AmbientLight = video::ColorF(0,0,0);
		m_Fogs.Clear();
	}

	m_CollectedRoot = nullptr;
}

////////////////////////////////////////////////////////////////////////////

void SceneRendererSimpleForward::EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	auto anySettings = m_PassSettings[ERenderPass::Any];
	auto settings = m_PassSettings[pass];
	settings.wireframe |= anySettings.wireframe;
	settings.disableCulling |= anySettings.disableCulling;
	bool useOverwrite = false;
	video::PipelineOverwrite overwrite;
	if(settings.wireframe) {
		overwrite.Enable(video::EPipelineSetting::DrawMode);
		overwrite.drawMode = video::EDrawMode::Wire;
		useOverwrite = true;
	}
	if(settings.disableCulling) {
		overwrite.Enable(video::EPipelineSetting::Culling);
		overwrite.culling = video::EFaceSide::None;
		useOverwrite = true;
	}
	if(useOverwrite)
		m_Renderer->PushPipelineOverwrite(overwrite, &token);
	m_SettingsActive = useOverwrite;
}

void SceneRendererSimpleForward::DisableOverwrite(video::PipelineOverwriteToken& token)
{
	if(m_SettingsActive) {
		m_Renderer->PopPipelineOverwrite(&token);
		m_SettingsActive = false;
	}
}

void SceneRendererSimpleForward::AddRenderEntry(Node* n, Renderable* r)
{
	bool isCulled = false;

	switch(r->GetRenderPass()) {
	case ERenderPass::SkyBox:
		m_SkyBoxList.EmplaceBack(n, r);
		break;
	case ERenderPass::Solid:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_SolidNodeList.EmplaceBack(n, r, isCulled);
		break;
	case ERenderPass::Transparent:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_TransparentNodeList.EmplaceBack(n, r, isCulled);
		break;
	case ERenderPass::Any:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_SolidNodeList.EmplaceBack(n, r, isCulled);
		m_TransparentNodeList.EmplaceBack(n, r, isCulled);
		break;
	default:
		break;
	}
}

void SceneRendererSimpleForward::AddCameraEntry(AbstractCamera* cam)
{
	m_Cameras.PushBack(cam);
}

void SceneRendererSimpleForward::AddLight(Light* l)
{
	m_Lights.PushBack(l);
}
void SceneRendererSimpleForward::AddFog(GlobalFog* l)
{
	m_Fogs.PushBack(l);
}

void SceneRendererSimpleForward::AddAmbient(GlobalAmbientLight* l)
{
	m_AmbientLight += l->GetColor();
}

bool SceneRendererSimpleForward::IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum)
{
	LUX_UNUSED(r);
	if(!m_Culling)
		return false;
	if(node->GetBoundingBox().IsEmpty())
		return false;
	return !frustum.IsBoxVisible(node->GetBoundingBox(), node->GetAbsoluteTransform());
}

void SceneRendererSimpleForward::DrawScene()
{
	// Check if a stencil buffer is available for shadow rendering
	bool drawStencilShadows = m_Attributes["drawStencilShadows"];
	if(drawStencilShadows) {
		if(m_Renderer->GetDriver()->GetConfig().zsFormat.sBits == 0) {
			log::Warning("Scene: Can't draw stencil shadows without stencilbuffer(Disabled shadow rendering).");
			drawStencilShadows = false;
			m_Attributes["drawStencilShadows"] = false;
		}
	}

	m_Culling = m_Attributes["culling"];

	core::Array<Light*> illuminating;
	core::Array<Light*> shadowCasting;
	core::Array<Light*> nonShadowCasting;
	SceneData sceneData(illuminating, shadowCasting);
	sceneData.activeCamera = m_ActiveCamera;
	sceneData.activeCameraNode = m_ActiveCameraNode;

	*m_Renderer->GetParam("camPos") = m_ActiveCameraNode->GetAbsolutePosition();

	//-------------------------------------------------------------------------
	// The lights
	*m_Renderer->GetParam("ambient") = m_AmbientLight;
	m_Renderer->ClearLights();

	int maxLightCount = m_Renderer->GetMaxLightCount();
	int maxShadowCastingCount = m_Attributes["maxShadowCasters"];
	if(!drawStencilShadows)
		maxShadowCastingCount = 0;

	int count = 0;
	int shadowCount = 0;
	for(auto& e : m_Lights) {
		illuminating.PushBack(e);
		if(e->IsShadowCasting() && shadowCount < maxShadowCastingCount) {
			shadowCasting.PushBack(e);
			++shadowCount;
		} else {
			nonShadowCasting.PushBack(e);
		}
		++count;
		if(count >= maxLightCount)
			break;
	}

	//-------------------------------------------------------------------------
	// The fog
	bool correctFogForStencilShadows = true;
	m_Renderer->ClearFog();
	if(m_Fogs.Size() > 0) {
		auto fog = m_Fogs[0]->GetFogData();
		m_Renderer->SetFog(fog);
	}
	if(m_Fogs.Size() > 1)
		log::Warning("Simple Forward Scene Renderer: Only support one fog element per scene.(all but one fog disabled)");

	video::PipelineOverwriteToken pot;

	//-------------------------------------------------------------------------
	// Skyboxes
	sceneData.pass = ERenderPass::SkyBox;
	EnableOverwrite(ERenderPass::SkyBox, pot);

	for(auto& e : m_SkyBoxList)
		e.renderable->Render(e.node, m_Renderer, sceneData);

	DisableOverwrite(pot); // SkyBox

	//-------------------------------------------------------------------------
	// Solid objects

	EnableOverwrite(ERenderPass::Solid, pot);

	if(drawStencilShadows) {
		video::PipelineOverwrite illumOver;
		illumOver.Enable(video::EPipelineSetting::Lighting);
		illumOver.lighting = video::ELightingFlag::AmbientEmit;
		m_Renderer->PushPipelineOverwrite(illumOver, &pot);
	} else {
		for(auto& e : illuminating)
			m_Renderer->AddLight(e->GetLightData());
	}

	// Ambient pass for shadows, otherwise the normal renderpass
	sceneData.pass = ERenderPass::Solid;
	for(auto& e : m_SolidNodeList) {
		if(!e.IsCulled())
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}
	DisableOverwrite(pot); // Solid

	// Stencil shadow rendering
	if(drawStencilShadows) {
		// To renderer correct fog, render black fog.
		if(correctFogForStencilShadows && m_Fogs.Size() > 0) {
			auto data = m_Fogs[0]->GetFogData();
			data.color = video::ColorF(0,0,0,0);
			m_Renderer->SetFog(data);
		}

		m_Renderer->PopPipelineOverwrite(&pot);

		// Shadow pass for each shadow casting light
		for(auto shadowLight : shadowCasting) {
			auto shadowNode = shadowLight->GetParent();
			m_Renderer->ClearLights();
			m_Renderer->AddLight(shadowLight->GetLightData());

			m_StencilShadowRenderer.Begin(m_ActiveCameraNode->GetAbsolutePosition(), m_ActiveCameraNode->GetAbsoluteTransform().TransformDir(math::Vector3F::UNIT_Y));
			for(auto& e : m_SolidNodeList) {
				if(e.node->IsShadowCasting()) {
					auto mesh = dynamic_cast<Mesh*>(e.renderable);
					if(mesh && mesh->GetMesh()) {
						bool isInfinite;
						math::Vector3F lightPos;
						if(shadowLight->GetLightType() == video::ELightType::Directional) {
							isInfinite = true;
							lightPos = e.node->ToRelativeDir(shadowNode->FromRelativeDir(math::Vector3F::UNIT_Z));
						} else {
							isInfinite = false;
							lightPos = e.node->ToRelativePos(shadowNode->GetAbsolutePosition());
						}
						m_StencilShadowRenderer.AddSilhouette(e.node->GetAbsoluteTransform(), mesh->GetMesh(), lightPos, isInfinite);
					}
				}
			}

			m_StencilShadowRenderer.End();

			video::PipelineOverwrite illumOver;
			illumOver.
				Enable(video::EPipelineSetting::ZWrite).
				Enable(video::EPipelineSetting::Lighting).
				Enable(video::EPipelineSetting::AlphaBlending).
				Enable(video::EPipelineSetting::Stencil);
			illumOver.zWriteEnabled = false;
			illumOver.lighting = video::ELightingFlag::DiffSpec;
			illumOver.alpha.blendOperator = video::EBlendOperator::Add;
			illumOver.alpha.srcFactor = video::EBlendFactor::One;
			illumOver.alpha.dstFactor = video::EBlendFactor::One;
			illumOver.stencil = m_StencilShadowRenderer.GetIllumniatedStencilMode();

			EnableOverwrite(ERenderPass::Solid, pot);
			m_Renderer->PushPipelineOverwrite(illumOver, &pot);

			for(auto& e : m_SolidNodeList) {
				if(!e.IsCulled())
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(pot); // Solid

			m_Renderer->Clear(false, false, true);
		}

		if(!nonShadowCasting.IsEmpty()) {
			m_Renderer->ClearLights();
			// Draw with remaining non shadow casting lights
			for(auto illum : nonShadowCasting)
				m_Renderer->AddLight(illum->GetLightData());

			video::PipelineOverwrite illumOver;
			illumOver.
				Enable(video::EPipelineSetting::ZWrite).
				Enable(video::EPipelineSetting::Lighting).
				Enable(video::EPipelineSetting::AlphaBlending);
			illumOver.zWriteEnabled = false;
			illumOver.lighting = video::ELightingFlag::DiffSpec;
			illumOver.alpha.blendOperator = video::EBlendOperator::Add;
			illumOver.alpha.srcFactor = video::EBlendFactor::One;
			illumOver.alpha.dstFactor = video::EBlendFactor::One;
			EnableOverwrite(ERenderPass::Solid, pot);
			m_Renderer->PushPipelineOverwrite(illumOver, &pot);

			sceneData.pass = ERenderPass::Solid;
			for(auto& e : m_SolidNodeList) {
				if(!e.IsCulled())
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(pot); // Solid
		}

		// Add shadow casting lights for remaining render jobs
		for(auto illum : shadowCasting)
			m_Renderer->AddLight(illum->GetLightData());

		// Restore correct fog
		if(correctFogForStencilShadows && m_Fogs.Size() > 0) {
			auto data = m_Fogs[0]->GetFogData();
			m_Renderer->SetFog(data);
		}
	}

	//-------------------------------------------------------------------------
	// Transparent objects
	EnableOverwrite(ERenderPass::Transparent, pot);

	sceneData.pass = ERenderPass::Transparent;
	// Update the distances in the transparent nodes
	for(auto& e : m_TransparentNodeList) {
		e.UpdateDistance(m_AbsoluteCamPos);
	}

	// Sort by distance
	m_TransparentNodeList.Sort();

	for(auto& e : m_TransparentNodeList) {
		if(!e.IsCulled())
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}

	DisableOverwrite(pot); // Transparent
}

} // namespace scene
} // namespace lux
