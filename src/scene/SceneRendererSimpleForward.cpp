#include "scene/SceneRendererSimpleForward.h"
#include "core/Logger.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "video/mesh/VideoMesh.h"

#include "core/lxAlgorithm.h"

#include "scene/components/SceneMesh.h"

/*
Lux illumination matrix.
Only the location of the t value is fixed, all other values depend on the light
type, for built-in point/directonal and spot light the matrix is build as
following:

 r  g  b t
px py pz 0
dx dy dz 0
ra ic oc 0

t = Type of light (0 = Disabled, 1 = Directional, 2 = Point, 3 = Spot)

(r,g,b) = Diffuse color of light
(px,py,pz) = Position of light

fa = Falloff for spotlight
ic = Cosine of half inner cone for spotlight
oc = Cosine of half outer cone for spotlight
*/
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
		alb.AddAttribute("fogA", video::ColorF(0, 0, 0, 0));
		alb.AddAttribute("fogB", video::ColorF(0, 0, 0, 0));
		alb.AddAttribute("light0", math::Matrix4());
		alb.AddAttribute("light1", math::Matrix4());
		alb.AddAttribute("light2", math::Matrix4());
		alb.AddAttribute("light3", math::Matrix4());

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

bool SceneRendererSimpleForward::IsCulled(Node* node, Renderable* r, const math::ViewFrustum& frustum)
{
	LUX_UNUSED(r);
	if(!m_Culling)
		return false;
	if(node->GetBoundingBox().IsEmpty())
		return false;
	return !frustum.IsBoxVisible(node->GetBoundingBox(), node->GetAbsoluteTransform());
}

static void SetFogData(video::Renderer* renderer, ClassicalFogDescription* desc, video::ColorF* overwriteColor = nullptr)
{
	if(desc) {
		video::ColorF fogB;
		video::ColorF fogA;
		fogA = overwriteColor ? *overwriteColor : desc->GetColor();
		renderer->GetParams()["fogA"].Set(fogA);
		auto type = desc->GetType();
		fogB.r =
			type == video::EFogType::Linear ? 1.0f :
			type == video::EFogType::Exp ? 2.0f :
			type == video::EFogType::ExpSq ? 3.0f : 0.0f;
		fogB.g = desc->GetStart();
		fogB.b = desc->GetEnd();
		fogB.a = desc->GetDensity();
		renderer->GetParams()["fogB"].Set(fogB);
	} else {
		video::ColorF fogB;
		fogB.r = 0.0f;
		renderer->GetParams()["fogB"].Set(fogB);
	}
}

static float LightTypeToFloat(video::ELightType type)
{
	switch(type) {
	case video::ELightType::Directional: return 1;
	case video::ELightType::Point: return 2;
	case video::ELightType::Spot: return 3;
	default:
		throw core::GenericInvalidArgumentException("type", "Unknown light data type");
	}
}


static math::Matrix4 GenerateLightMatrix(ClassicalLightDescription* desc)
{
	math::Matrix4 matrix;

	if(!desc) {
		matrix(0, 3) = 0.0f;
		return matrix;
	}
	matrix(0, 3) = LightTypeToFloat(desc->GetType());

	auto color = desc->GetColor();
	matrix(0, 0) = color.r;
	matrix(0, 1) = color.g;
	matrix(0, 2) = color.b;

	auto position = desc->GetPosition();
	matrix(1, 0) = position.x;
	matrix(1, 1) = position.y;
	matrix(1, 2) = position.z;

	auto direction = desc->GetDirection();
	matrix(2, 0) = direction.x;
	matrix(2, 1) = direction.y;
	matrix(2, 2) = direction.z;

	matrix(3, 0) = desc->GetFalloff();
	matrix(3, 1) = cos(desc->GetInnerCone());
	matrix(3, 2) = cos(desc->GetOuterCone());
	matrix(3, 3) = 0.0f;

	matrix(1, 3) = 0.0f;
	matrix(2, 3) = 0.0f;

	return matrix;
}

void SceneRendererSimpleForward::ClearLightData(video::Renderer* renderer)
{
	m_CurLightId = 0;
	renderer->GetParams()["light0"].Set(GenerateLightMatrix(nullptr));
	renderer->GetParams()["light1"].Set(GenerateLightMatrix(nullptr));
	renderer->GetParams()["light2"].Set(GenerateLightMatrix(nullptr));
	renderer->GetParams()["light3"].Set(GenerateLightMatrix(nullptr));
}

void SceneRendererSimpleForward::AddLightData(video::Renderer* renderer, ClassicalLightDescription* desc)
{
	if(m_CurLightId < m_MaxLightsPerDraw) {
		core::String name;
		format::format(name, &format::InvariantLocale, "light{}", m_CurLightId);
		renderer->GetParams()[name].Set(GenerateLightMatrix(desc));
	}
	++m_CurLightId;
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

	struct LightEntry
	{
		ClassicalLightDescription* desc;
		Node* node;
	};
	core::Array<LightEntry> illuminating;
	core::Array<LightEntry> shadowCasting;
	core::Array<LightEntry> nonShadowCasting;
	SceneData sceneData;
	sceneData.activeCamera = m_ActiveCamera;
	sceneData.activeCameraNode = m_ActiveCameraNode;

	m_Renderer->GetParams()["camPos"].Set(m_ActiveCameraNode->GetAbsolutePosition());

	//-------------------------------------------------------------------------
	// The lights
	ClearLightData(m_Renderer);
	video::ColorF totalAmbientLight;
	int maxShadowCastingCount = m_Attributes["maxShadowCasters"].Get<int>();
	if(!drawStencilShadows)
		maxShadowCastingCount = 0;
	maxShadowCastingCount = math::Clamp(maxShadowCastingCount, 0, m_MaxLightsPerDraw);

	int count = 0;
	int shadowCount = 0;
	for(auto& e : m_Lights) {
		auto descBase = e->GetLightDescription();
		if(auto descC = dynamic_cast<ClassicalLightDescription*>(descBase)) {
			LightEntry entry{descC, e->GetParent()};
			illuminating.PushBack(entry);
			if(descC->IsShadowCasting() && shadowCount < maxShadowCastingCount) {
				shadowCasting.PushBack(entry);
				++shadowCount;
			} else {
				nonShadowCasting.PushBack(entry);
			}
			++count;
			if(count >= m_MaxLightsPerDraw)
				break;
		} else if(auto descA = dynamic_cast<AmbientLightDescription*>(descBase)) {
			totalAmbientLight += descA->GetColor();
		}
	}
	m_Renderer->GetParams()["ambient"].Set(totalAmbientLight);

	//-------------------------------------------------------------------------
	// The fog
	bool correctFogForStencilShadows = true;
	ClassicalFogDescription* fog = nullptr;
	for(auto& f : m_Fogs) {
		auto fogDesc = dynamic_cast<ClassicalFogDescription*>(f->GetFogDescription());
		if(fogDesc) {
			if(!fog)
				fog = fogDesc;
			else
				log::Warning("Simple Forward Scene Renderer: Only support one fog element per scene.(all but one fog disabled)");
		}
	}
	if(fog)
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
			AddLightData(m_Renderer, e.desc);
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
		if(correctFogForStencilShadows && fog) {
			auto overwriteColor = video::ColorF(0, 0, 0, 0);
			SetFogData(m_Renderer, fog, &overwriteColor);
		}

		m_Renderer->PopPipelineOverwrite(&pot);

		// Shadow pass for each shadow casting light
		for(auto shadowEntry : shadowCasting) {
			auto shadowLight = shadowEntry.desc;
			auto shadowNode = shadowEntry.node;
			ClearLightData(m_Renderer);
			AddLightData(m_Renderer, shadowLight);

			m_StencilShadowRenderer.Begin(m_ActiveCameraNode->GetAbsolutePosition(), m_ActiveCameraNode->GetAbsoluteTransform().TransformDir(math::Vector3F::UNIT_Y));
			for(auto& e : m_SolidNodeList) {
				if(e.node->IsShadowCasting()) {
					auto mesh = dynamic_cast<Mesh*>(e.renderable);
					if(mesh && mesh->GetMesh()) {
						bool isInfinite;
						math::Vector3F lightPos;
						if(shadowLight->GetType() == video::ELightType::Directional) {
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
			ClearLightData(m_Renderer);
			// Draw with remaining non shadow casting lights
			for(auto illum : nonShadowCasting)
				AddLightData(m_Renderer, illum.desc);

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
			AddLightData(m_Renderer, illum.desc);

		// Restore correct fog
		if(correctFogForStencilShadows && fog)
			SetFogData(m_Renderer, fog);
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
