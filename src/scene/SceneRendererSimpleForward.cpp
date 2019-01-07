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

namespace lux
{
namespace scene
{

SceneRendererSimpleForward::SceneRendererSimpleForward(const scene::SceneRendererInitData& data) :
	m_CollectedRoot(nullptr),
	m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF)
{
	m_Renderer = video::VideoDriver::Instance()->GetRenderer();
	m_Scene = data.scene;

	{
		core::AttributeListBuilder alb;
		alb.AddAttribute("drawStencilShadows", false);
		alb.AddAttribute("maxShadowCasters", 1);
		alb.AddAttribute("culling", true);
		m_Attributes = alb.BuildAndReset();
	}

	{
		core::AttributeListBuilder alb;
		alb.SetBase(m_Renderer->GetBaseParams());
		alb.AddAttribute("camPos", math::Vector3F(0, 0, 0));
		alb.AddAttribute("fog1", video::ColorF(0, 0, 0, 0));
		alb.AddAttribute("fog2", video::ColorF(0, 0, 0, 0));

		m_RendererAttributes = alb.BuildAndReset();
	}
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
		auto fog = dynamic_cast<Fog*>(comp);
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
	m_Renderer->SetParams(m_RendererAttributes);

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
		m_AmbientLight = video::ColorF(0, 0, 0);
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
		overwrite.OverwriteDrawMode(video::EDrawMode::Wire);
		useOverwrite = true;
	}
	if(settings.disableCulling) {
		overwrite.OverwriteCulling(video::EFaceSide::None);
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
void SceneRendererSimpleForward::AddFog(Fog* l)
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

static void SetFogData(video::Renderer* renderer, FogDescription* desc, video::ColorF* overwriteColor=nullptr)
{
	if(desc) {
		video::ColorF fog2;
		video::ColorF fog1;
		fog1 = overwriteColor ? *overwriteColor : desc->GetColor();
		renderer->GetParams()["fog1"].Set(fog1);
		auto type = desc->GetType();
		fog2.r =
			type == video::EFogType::Linear ? 1.0f :
			type == video::EFogType::Exp ? 2.0f :
			type == video::EFogType::ExpSq ? 3.0f : 0.0f;
		fog2.g = desc->GetStart();
		fog2.b = desc->GetEnd();
		fog2.a = desc->GetDensity();
		renderer->GetParams()["fog2"].Set(fog2);
	} else {
		video::ColorF fog2;
		fog2.r = 0.0f;
		renderer->GetParams()["fog2"].Set(fog2);
	}
}

void SceneRendererSimpleForward::DrawScene()
{
	// Check if a stencil buffer is available for shadow rendering
	bool drawStencilShadows = m_Attributes["drawStencilShadows"].Get<bool>();
	if(drawStencilShadows) {
		if(m_Renderer->GetDriver()->GetConfig().zsFormat.sBits == 0) {
			log::Warning("Scene: Can't draw stencil shadows without stencilbuffer(Disabled shadow rendering).");
			drawStencilShadows = false;
			m_Attributes["drawStencilShadows"].Set(false);
		}
	}

	m_Culling = m_Attributes["culling"].Get<bool>();

	core::Array<Light*> illuminating;
	core::Array<Light*> shadowCasting;
	core::Array<Light*> nonShadowCasting;
	SceneData sceneData(illuminating, shadowCasting);
	sceneData.activeCamera = m_ActiveCamera;
	sceneData.activeCameraNode = m_ActiveCameraNode;

	m_Renderer->GetParams()["camPos"].Set(m_ActiveCameraNode->GetAbsolutePosition());

	//-------------------------------------------------------------------------
	// The lights
	m_Renderer->GetParams()["ambient"].Set(m_AmbientLight);
	m_Renderer->ClearLights();

	int maxLightCount = m_Renderer->GetMaxLightCount();
	int maxShadowCastingCount = m_Attributes["maxShadowCasters"].Get<int>();
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
	auto fog = m_Fogs.Size() > 0 ? m_Fogs[0]->GetFogDescription() : nullptr;
	if(m_Fogs.Size() > 1)
		log::Warning("Simple Forward Scene Renderer: Only support one fog element per scene.(all but one fog disabled)");
	SetFogData(m_Renderer, fog);

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
		illumOver.OverwriteLighting(video::ELightingFlag::AmbientEmit);
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
			fog = m_Fogs[0]->GetFogDescription();
			auto overwriteColor = video::ColorF(0, 0, 0, 0);
			SetFogData(m_Renderer, fog, &overwriteColor);
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
			illumOver.OverwriteZWrite(false);
			illumOver.OverwriteLighting(video::ELightingFlag::DiffSpec);
			illumOver.OverwriteAlpha(video::AlphaBlendMode(video::EBlendFactor::One, video::EBlendFactor::One, video::EBlendOperator::Add));
			illumOver.OverwriteStencil(m_StencilShadowRenderer.GetIllumniatedStencilMode());
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

			EnableOverwrite(ERenderPass::Solid, pot);

			video::PipelineOverwrite illumOver;
			illumOver.OverwriteZWrite(false);
			illumOver.OverwriteLighting(video::ELightingFlag::DiffSpec);
			illumOver.OverwriteAlpha(video::AlphaBlendMode(video::EBlendFactor::One, video::EBlendFactor::One, video::EBlendOperator::Add));
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
			fog = m_Fogs[0]->GetFogDescription();
			SetFogData(m_Renderer, fog);
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
