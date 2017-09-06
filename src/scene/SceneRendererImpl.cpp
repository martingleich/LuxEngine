#include "scene/SceneRendererImpl.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

//#include "video/MaterialLibrary.h"
//#include "video/images/ImageSystem.h"
#include "video/mesh/MeshSystem.h"
#include "video/mesh/VideoMesh.h"

#include "core/lxAlgorithm.h"

#include "core/Logger.h"

#include "scene/Scene.h"
#include "scene/components/Fog.h"
#include "scene/components/Light.h"
#include "scene/components/SceneMesh.h"

//#include "video/VertexTypes.h"

namespace lux
{
namespace scene
{

SceneRendererImpl::SceneRendererImpl() :
	m_CollectedRoot(nullptr),
	m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF)
{
	m_Renderer = video::VideoDriver::Instance()->GetRenderer();

	m_Attributes.AddAttribute("drawStencilShadows", false);
	m_Attributes.AddAttribute("maxShadowCasters", 1);
	m_Attributes.AddAttribute("culling", true);
}

SceneRendererImpl::~SceneRendererImpl()
{
}

////////////////////////////////////////////////////////////////////////////////////

struct CameraSortT
{
	bool Smaller(const Camera* a, const Camera* b) const
	{
		return a->GetRenderPriority() < b->GetRenderPriority();
	}
};
void SceneRendererImpl::DrawScene(Scene* scene, bool beginScene, bool endScene)
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
		if(beginScene == true)
			m_Renderer->BeginScene(true, true, true, video::Color::Black, 1.0f, 0);
		if(endScene)
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
			m_Renderer->BeginScene(clearColor, clearZ, clearStencil,
				video::Color::Black, 1.0f, 0);
		}

		m_ActiveCamera->PreRender(m_Renderer);

		m_ActiveCamera->Render(m_Renderer);

		CollectRenderables(m_Scene->GetRoot());
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

////////////////////////////////////////////////////////////////////////////////////

void SceneRendererImpl::PushPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over)
{
	m_Overwrites[pass].PushBack(over);
}

void SceneRendererImpl::PopPipelineOverwrite(ERenderPass pass)
{
	m_Overwrites[pass].PopBack();
}

////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////

void SceneRendererImpl::EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	for(auto& over : m_Overwrites[pass])
		m_Renderer->PushPipelineOverwrite(over, &token);
}

void SceneRendererImpl::DisableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	for(auto& over : m_Overwrites[pass]) {
		LUX_UNUSED(over);
		m_Renderer->PopPipelineOverwrite(&token);
	}
}

class SceneRendererImpl::RenderableCollector : public RenderableVisitor
{
public:
	void Visit(Renderable* r)
	{
		smgr->AddRenderEntry(curNode, r);
	}

	Node* curNode;
	SceneRendererImpl* smgr;
};

void SceneRendererImpl::CollectRenderables(Node* root)
{
	if(root == m_CollectedRoot)
		return;

	m_SkyBoxList.Clear();
	m_SolidNodeList.Clear();
	m_TransparentNodeList.Clear();

	RenderableCollector collector;
	collector.smgr = this;
	CollectRenderablesRec(root, &collector, true);
	m_CollectedRoot = root;
}

void SceneRendererImpl::CollectRenderablesRec(Node* node, RenderableCollector* collector, bool noDebug)
{
	collector->curNode = node;
	node->VisitRenderables(collector, noDebug);

	for(auto child : node->Children())
		CollectRenderablesRec(child, collector, noDebug);
}

void SceneRendererImpl::AddRenderEntry(Node* n, Renderable* r)
{
	if(!n->IsTrulyVisible())
		return;

	bool isCulled = false;

	switch(r->GetRenderPass()) {
	case ERenderPass::SkyBox:
		m_SkyBoxList.PushBack(RenderEntry(n, r));
		break;
	case ERenderPass::Solid:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_SolidNodeList.PushBack(RenderEntry(n, r, isCulled));
		break;
	case ERenderPass::Transparent:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r, isCulled));
		break;
	case ERenderPass::SolidAndTransparent:
		isCulled = IsCulled(n, r, m_ActiveCamera->GetActiveFrustum());
		m_SolidNodeList.PushBack(RenderEntry(n, r, isCulled));
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r, isCulled));
		break;
	default:
		break;
	}
}

bool SceneRendererImpl::IsCulled(const RenderEntry& e)
{
	return e.isCulled;
}

bool SceneRendererImpl::IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum)
{
	LUX_UNUSED(r);
	if(!m_Culling)
		return false;
	if(node->GetBoundingBox().IsEmpty())
		return false;
	return !frustum.IsBoxVisible(node->GetBoundingBox(), node->GetAbsoluteTransform());
}

void SceneRendererImpl::DrawScene()
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

	core::Array<SceneData::LightEntry> illuminating;
	core::Array<SceneData::LightEntry> shadowCasting;
	core::Array<SceneData::LightEntry> nonShadowCasting;
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
			illuminating.PushBack(SceneData::LightEntry(e, node));
			if(e->IsShadowCasting() && shadowCount < maxShadowCastingCount) {
				shadowCasting.PushBack(SceneData::LightEntry(e, node));
				++shadowCount;
			} else {
				nonShadowCasting.PushBack(SceneData::LightEntry(e, node));
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

	EnableOverwrite(ERenderPass::SolidAndTransparent, pot);
	EnableOverwrite(ERenderPass::Solid, pot);

	if(drawStencilShadows) {
		video::PipelineOverwrite illumOver;
		illumOver.lighting = video::ELighting::AmbientEmit;
		m_Renderer->PushPipelineOverwrite(illumOver, &pot);
	} else {
		for(auto& e : illuminating)
			AddDriverLight(e.node, e.light);
	}

	// Ambient pass for shadows, otherwise the normal renderpass
	sceneData.pass = ERenderPass::Solid;
	for(auto& e : m_SolidNodeList) {
		if(!IsCulled(e))
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}
	DisableOverwrite(ERenderPass::Solid, pot);

	// Stencil shadow rendering
	if(drawStencilShadows) {
		m_Renderer->PopPipelineOverwrite(&pot);

		// Shadow pass for each shadow casting light
		for(auto shadowLight : shadowCasting) {
			m_Renderer->ClearLights();
			AddDriverLight(shadowLight.node, shadowLight.light);

			m_StencilShadowRenderer.Begin(m_ActiveCameraNode->GetAbsolutePosition(), m_ActiveCameraNode->GetAbsoluteTransform().TransformDir(math::Vector3F::UNIT_Y));
			for(auto& e : m_SolidNodeList) {
				if(e.node->IsShadowCasting()) {
					auto mesh = dynamic_cast<Mesh*>(e.renderable);
					if(mesh && mesh->GetMesh()) {
						bool isInfinite;
						math::Vector3F lightPos;
						if(shadowLight.light->GetLightType() == video::ELightType::Directional) {
							isInfinite = true;
							lightPos = e.node->ToRelativeDir(shadowLight.node->FromRelativeDir(math::Vector3F::UNIT_Z));
						} else {
							isInfinite = false;
							lightPos = e.node->ToRelativePos(shadowLight.node->GetAbsolutePosition());
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
				if(!IsCulled(e))
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(ERenderPass::Solid, pot);

			m_Renderer->ClearStencil();
		}

		if(!nonShadowCasting.IsEmpty()) {
			m_Renderer->ClearLights();
			// Draw with remaining non shadow casting lights
			for(auto illum : nonShadowCasting)
				AddDriverLight(illum.node, illum.light);

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
				if(!IsCulled(e))
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(ERenderPass::Solid, pot);
		}
		// Add shadow casting light for remaining render jobs
		for(auto illum : shadowCasting)
			AddDriverLight(illum.node, illum.light);
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
		if(!IsCulled(e))
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}

	DisableOverwrite(ERenderPass::Transparent, pot);
	DisableOverwrite(ERenderPass::SolidAndTransparent, pot);
}

void SceneRendererImpl::AddDriverLight(Node* n, Light* l)
{
	auto data = l->GetLightData();
	if(data.type == video::ELightType::Spot ||
		data.type == video::ELightType::Directional) {
		data.direction = n->FromRelativeDir(math::Vector3F::UNIT_Z);
	}

	if(data.type == video::ELightType::Spot ||
		data.type == video::ELightType::Point) {
		data.position = n->GetAbsolutePosition();
	}

	m_Renderer->AddLight(data);
}

} // namespace scene
} // namespace lux
