#include "scene/SceneRendererSimpleForward.h"
#include "core/Logger.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "video/mesh/VideoMesh.h"

#include "core/lxAlgorithm.h"

#include "scene/Scene.h"
#include "scene/components/Fog.h"
#include "scene/components/Light.h"
#include "scene/components/SceneMesh.h"

#include "core/ModuleFactoryRegister.h"

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

	m_Attributes.AddAttribute("drawStencilShadows", false);
	m_Attributes.AddAttribute("maxShadowCasters", 1);
	m_Attributes.AddAttribute("culling", true);
}

////////////////////////////////////////////////////////////////////////////////////

namespace
{
class RenderableCollector : public RenderableVisitor
{
public:
	RenderableCollector(SceneRendererSimpleForward* _sr) :
		sr(_sr)
	{}

	void Visit(Node* node, Renderable* r)
	{
		sr->AddRenderEntry(node, r);
	}

	SceneRendererSimpleForward* sr;
};

struct CameraSortT
{
	bool Smaller(const Camera* a, const Camera* b) const
	{
		return a->GetRenderPriority() < b->GetRenderPriority();
	}
};
}

void SceneRendererSimpleForward::DrawScene(Scene* scene, bool beginScene, bool endScene)
{
	video::RenderStatistics::GroupScope grpScope("scene");
	m_Scene = scene;

	// Collect "real" camera nodes
	auto camList = m_Scene->GetCameraList();
	auto newEnd = core::RemoveIf(camList,
		[](const Camera* c) { return !c->GetParent()->IsTrulyVisible(); }
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
		m_Scene->VisitRenderables(&collector, true);
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
// Private functions
////////////////////////////////////////////////////////////////////////////

void SceneRendererSimpleForward::EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	// Enable debug things like wire frame
	LUX_UNUSED(pass);
	LUX_UNUSED(token);
}

void SceneRendererSimpleForward::DisableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	// Disable debug things like wire frame
	LUX_UNUSED(pass);
	LUX_UNUSED(token);
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
	*m_Renderer->GetParam("ambient") = m_Scene->GetAmbient();

	m_Renderer->ClearLights();

	size_t maxLightCount = m_Renderer->GetMaxLightCount();
	size_t maxShadowCastingCount = drawStencilShadows ? m_Attributes["maxShadowCasters"] : 0;

	size_t count = 0;
	size_t shadowCount = 0;
	for(auto& e : m_Scene->GetLightList()) {
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
	for(auto& f : m_Scene->GetFogList()) {
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

	DisableOverwrite(ERenderPass::SkyBox, pot);

	//-------------------------------------------------------------------------
	// Solid objects

	EnableOverwrite(ERenderPass::Any, pot);
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
	DisableOverwrite(ERenderPass::Solid, pot);

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
			illumOver.disableZWrite = true;
			illumOver.lighting = video::ELighting::DiffSpec;
			illumOver.useAlphaOverwrite = true;
			illumOver.alphaOperator = video::EBlendOperator::Add;
			illumOver.alphaSrcBlend = video::EBlendFactor::One;
			illumOver.alphaDstBlend = video::EBlendFactor::One;
			illumOver.useStencilOverwrite = true;
			illumOver.stencil = m_StencilShadowRenderer.GetIllumniatedStencilMode();

			EnableOverwrite(ERenderPass::Solid, pot);
			m_Renderer->PushPipelineOverwrite(illumOver, &pot);

			for(auto& e : m_SolidNodeList) {
				if(!e.IsCulled())
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(ERenderPass::Solid, pot);

			m_Renderer->Clear(false, false, true);
		}

		if(!nonShadowCasting.IsEmpty()) {
			m_Renderer->ClearLights();
			// Draw with remaining non shadow casting lights
			for(auto illum : nonShadowCasting)
				m_Renderer->AddLight(illum->GetLightData());

			video::PipelineOverwrite illumOver;
			illumOver.disableZWrite = true;
			illumOver.lighting = video::ELighting::DiffSpec;
			illumOver.useAlphaOverwrite = true;
			illumOver.alphaOperator = video::EBlendOperator::Add;
			illumOver.alphaSrcBlend = video::EBlendFactor::One;
			illumOver.alphaDstBlend = video::EBlendFactor::One;
			EnableOverwrite(ERenderPass::Solid, pot);
			m_Renderer->PushPipelineOverwrite(illumOver, &pot);

			sceneData.pass = ERenderPass::Solid;
			for(auto& e : m_SolidNodeList) {
				if(!e.IsCulled())
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(ERenderPass::Solid, pot);
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

	DisableOverwrite(ERenderPass::Transparent, pot);
	DisableOverwrite(ERenderPass::Any, pot);
}

} // namespace scene
} // namespace lux
