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

#include "core/ModuleFactoryRegister.h"

LUX_REGISTER_MODULE("SceneRenderer", "SimpleForward", lux::scene::SceneRendererSimpleForward)

namespace lux
{
namespace scene
{

void SceneDataCollector::RegisterCamera(AbstractCamera* camera)
{
	cameraList.PushBack(camera);
}

void SceneDataCollector::UnregisterCamera(AbstractCamera* camera)
{
	auto it = core::LinearSearch(camera, cameraList);
	if(it != cameraList.End())
		cameraList.Erase(it);
}

void SceneDataCollector::RegisterLight(Light* light)
{
	lightList.PushBack(light);
}

void SceneDataCollector::UnregisterLight(Light* light)
{
	auto it = core::LinearSearch(light, lightList);
	if(it != lightList.End())
		lightList.Erase(it);
}

void SceneDataCollector::RegisterFog(GlobalFog* fog)
{
	fogList.PushBack(fog);
}

void SceneDataCollector::UnregisterFog(GlobalFog* fog)
{
	auto it = core::LinearSearch(fog, fogList);
	if(it != fogList.End())
		fogList.Erase(it);
}

void SceneDataCollector::OnAttach(Node* node)
{
	for(auto c : node->Children())
		OnAttach(c);
	for(auto c : node->Components())
		OnAttach(c);
}

void SceneDataCollector::OnDetach(Node* node)
{
	for(auto c : node->Children())
		OnDetach(c);
	for(auto c : node->Components())
		OnDetach(c);
}

void SceneDataCollector::OnAttach(Component* comp)
{
	auto fog = dynamic_cast<GlobalFog*>(comp);
	if(fog)
		RegisterFog(fog);
	auto light = dynamic_cast<Light*>(comp);
	if(light)
		RegisterLight(light);
	auto camera = dynamic_cast<AbstractCamera*>(comp);
	if(camera)
		RegisterCamera(camera);
	auto ambient = dynamic_cast<GlobalAmbientLight*>(comp);
	if(ambient)
		ambientLight = ambient;
}

void SceneDataCollector::OnDetach(Component* comp)
{
	auto fog = dynamic_cast<GlobalFog*>(comp);
	if(fog)
		UnregisterFog(fog);
	auto light = dynamic_cast<Light*>(comp);
	if(light)
		UnregisterLight(light);
	auto camera = dynamic_cast<AbstractCamera*>(comp);
	if(camera)
		UnregisterCamera(camera);
	auto ambient = dynamic_cast<GlobalAmbientLight*>(comp);
	if(ambient)
		ambientLight = nullptr;
}

SceneRendererSimpleForward::SceneRendererSimpleForward(const core::ModuleInitData& data) :
	m_CollectedRoot(nullptr),
	m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF)
{
	LUX_UNUSED(data);

	m_Renderer = video::VideoDriver::Instance()->GetRenderer();
	m_Scene = dynamic_cast<const scene::SceneRendererInitData&>(data).scene;
	m_SceneData = LUX_NEW(SceneDataCollector);

	m_Attributes.AddAttribute("drawStencilShadows", false);
	m_Attributes.AddAttribute("maxShadowCasters", 1);
	m_Attributes.AddAttribute("culling", true);

	m_Scene->RegisterObserver(m_SceneData);
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

struct CameraSortT
{
	bool Smaller(const AbstractCamera* a, const AbstractCamera* b) const
	{
		return a->GetRenderPriority() < b->GetRenderPriority();
	}
};
}

void SceneRendererSimpleForward::DrawScene(bool beginScene, bool endScene)
{
	video::RenderStatistics::GroupScope grpScope("scene");

	// Collect "real" camera nodes
	auto camList = m_SceneData->cameraList;
	auto newEnd = core::RemoveIf(camList,
		[](const AbstractCamera* c) { return !c->GetParent()->IsTrulyVisible(); }
	);
	camList.Resize(core::IteratorDistance(camList.First(), newEnd));

	camList.Sort(CameraSortT());

	if(camList.Size() == 0) {
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
		if(m_Renderer->GetRenderTarget() != camList[0]->GetRenderTarget())
			throw core::ErrorException("Already started scene uses diffrent rendertarget than first camera.");
	}

	for(auto it = camList.First(); it != camList.End(); ++it) {
		m_ActiveCamera = *it;
		m_ActiveCameraNode = m_ActiveCamera->GetParent();
		m_AbsoluteCamPos = m_ActiveCameraNode->GetAbsolutePosition();

		if(it != camList.First() || beginScene) {
			// Start a new scene
			m_Renderer->SetRenderTarget(m_ActiveCamera->GetRenderTarget());
			bool clearColor = true;
			bool clearZ = true;
			bool clearStencil = true;
			if(!m_SkyBoxList.IsEmpty()) {
				clearZ = true;
				clearColor = true;
			}
			m_Renderer->Clear(clearColor, clearZ, clearStencil);
			m_Renderer->BeginScene();
		}

		m_ActiveCamera->PreRender(m_Renderer);

		m_ActiveCamera->Render(m_Renderer);

		m_SkyBoxList.Clear();
		m_SolidNodeList.Clear();
		m_TransparentNodeList.Clear();
		RenderableCollector collector(this);
		m_Scene->VisitRenderables(&collector, ERenderableTags::None);
		DrawScene();

		m_ActiveCamera->PostRender(m_Renderer);

		if(it != camList.Last() || endScene)
			m_Renderer->EndScene();
	}

	m_SkyBoxList.Clear();
	m_SolidNodeList.Clear();
	m_TransparentNodeList.Clear();

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
	if(!n->IsTrulyVisible())
		return;

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
	*m_Renderer->GetParam("ambient") = m_SceneData->ambientLight ?
		m_SceneData->ambientLight->GetColor() :
		video::ColorF(0, 0, 0);

	m_Renderer->ClearLights();

	size_t maxLightCount = m_Renderer->GetMaxLightCount();
	size_t maxShadowCastingCount = m_Attributes["maxShadowCasters"];
	if(!drawStencilShadows)
		maxShadowCastingCount = 0;

	size_t count = 0;
	size_t shadowCount = 0;
	for(auto& e : m_SceneData->lightList) {
		auto node = e->GetParent();
		if(node->IsTrulyVisible()) {
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
	}

	size_t passCount;
	if(drawStencilShadows) {
		passCount = 1 + shadowCasting.Size();
		if(!nonShadowCasting.IsEmpty())
			++passCount;
	} else {
		passCount = 1;
	}

	//-------------------------------------------------------------------------
	// The fog
	m_Renderer->ClearFog();
	bool foundFog = false;
	for(auto& f : m_SceneData->fogList) {
		auto node = f->GetParent();
		if(node->IsTrulyVisible()) {
			if(!foundFog) {
				/*
				if(drawStencilShadows) {
					log::Warning("Scenerenderer can't draw fog while using stencil shadows.(fog disabled)");
					break;
				}
				*/
				foundFog = true;
				auto fog = f->GetFogData();
				if(passCount > 1)
					fog.color *= 1.0f / passCount;
				m_Renderer->SetFog(fog);
			} else {
				log::Warning("Scenerenderer only supports one fog element per scene.(all but one fog disabled)");
				break;
			}
		}
	}

	video::PipelineOverwriteToken pot;

	//-------------------------------------------------------------------------
	// Skyboxes
	sceneData.pass = ERenderPass::SkyBox;
	EnableOverwrite(ERenderPass::SkyBox, pot);

	for(auto& e : m_SkyBoxList) {
		e.renderable->Render(e.node, m_Renderer, sceneData);
	}

	DisableOverwrite(pot); // SkyBox

	//-------------------------------------------------------------------------
	// Solid objects

	EnableOverwrite(ERenderPass::Solid, pot);

	if(drawStencilShadows) {
		video::PipelineOverwrite illumOver;
		illumOver.lighting = video::ELighting::AmbientEmit;
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
			illumOver.lighting = video::ELighting::DiffSpec;
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
			illumOver.lighting = video::ELighting::DiffSpec;
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
		// Add shadow casting light for remaining render jobs
		for(auto illum : shadowCasting)
			m_Renderer->AddLight(illum->GetLightData());
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
