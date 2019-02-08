#include "scene/Scene.h"

#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"

#include "scene/Node.h"

#include "core/Logger.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "video/mesh/VideoMesh.h"

#include "core/lxAlgorithm.h"

#include "scene/components/SceneMesh.h"

#include "scene/StencilShadowRenderer.h"

namespace lux
{
namespace scene
{

class InternalRenderData
{
public:
	struct RenderEntry
	{
	public:
		Component* renderable;

		explicit RenderEntry(Component* r) :
			renderable(r)
		{
		}
	};

	struct DistanceRenderEntry : public RenderEntry
	{
		float distance;

		DistanceRenderEntry(Component* r, float _distance) :
			RenderEntry(r),
			distance(_distance)
		{
		}

		bool operator<(const DistanceRenderEntry& other) const
		{
			// The farthest element must be first in list
			return distance > other.distance;
		}
	};

public:
	InternalRenderData(Scene* scene, core::AttributeList* attributes) :
		m_Scene(scene),
		m_SceneAttributes(attributes),
		m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF),
		m_Renderer(video::VideoDriver::Instance()->GetRenderer())
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

	void AddRenderEntry(Node* n, Component* r);
	void ClearLightData(video::Renderer* renderer);
	void AddLightData(video::Renderer* renderer, ClassicalLightDescription* desc);

	// TODO: Pass attributes
	// TODO: Pass pass options
	// TODO: Pass scene data(cameras, fogs, etc.)

	void EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token);
	void DisableOverwrite(video::PipelineOverwriteToken& token);

	void DrawScene(bool beginScene, bool endScene);
	void DrawScene();
	bool IsCulled(Node* node, Component* r, const math::ViewFrustum& frustum);

public:
	Scene* m_Scene;
	core::AttributeList* m_SceneAttributes;

	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////

	core::Array<RenderEntry> m_SkyBoxList;
	core::Array<RenderEntry> m_SolidNodeList;
	core::Array<RenderEntry> m_ShadowCasters;
	core::Array<DistanceRenderEntry> m_TransparentNodeList;

	// Directly written by the Scene.
	core::HashSet<Component*> m_LightComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_FogComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_CamComps; //!< The animated nodes of the graph

	core::HashMap<ERenderPass, SceneRendererPassSettings> m_PassSettings;
	core::Array<AbstractCamera*> m_Cameras;
	core::Array<Light*> m_Lights;
	core::Array<Fog*> m_Fogs;

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
	core::AttributeList m_RendererAttributes;

	bool m_SettingsActive;

	/////////////////////////////////////////////////////////////////////////
	// References to other classes
	/////////////////////////////////////////////////////////////////////////

	video::Renderer* m_Renderer;
	StencilShadowRenderer m_StencilShadowRenderer;
};

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

////////////////////////////////////////////////////////////////////////////////////

Scene::Scene() :
	m_Root(LUX_NEW(Node)(this)),
	renderData(new InternalRenderData(this, &m_Attributes))
{

	{
		core::AttributeListBuilder alb;
		alb.AddAttribute("drawStencilShadows", false);
		alb.AddAttribute("maxShadowCasters", 1);
		alb.AddAttribute("culling", true);
		m_Attributes = alb.BuildAndReset();
	}
}

Scene::~Scene()
{
	// Explicitly free all nodes
	m_Root = nullptr;
	ClearDeletionQueue();
}

namespace
{
class RenderableCollector : public ComponentVisitor
{
public:
	RenderableCollector(InternalRenderData* _sr) :
		sr(_sr)
	{
	}

	void Visit(Component* c)
	{
		if(c->GetNode()->IsTrulyVisible())
			sr->AddRenderEntry(c->GetNode(), c);
	}

	InternalRenderData* sr;
};
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AddToDeletionQueue(Node* node)
{
	m_NodeDeletionQueue.PushBack(node);
}

void Scene::AddToDeletionQueue(Component* comp)
{
	m_CompDeletionQueue.PushBack(comp);
}

void Scene::ClearDeletionQueue()
{
	for(auto n : m_NodeDeletionQueue) {
		if(!n)
			continue;
		auto p = n->GetParent();
		if(!p)
			continue;
		p->RemoveChild(n);
	}
	m_NodeDeletionQueue.Clear();

	for(auto c : m_CompDeletionQueue) {
		if(!c)
			continue;
		auto p = c->GetNode();
		if(!p)
			continue;
		p->RemoveComponent(c);
	}
	m_CompDeletionQueue.Clear();
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Node> Scene::AddNode(Component* baseComp, Node* parent)
{
	if(!parent)
		parent = GetRoot();

	auto node = parent->AddChild();
	if(baseComp)
		node->AddComponent(baseComp);

	return node;
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterForTick(Component* c, bool doRegister)
{
	if(doRegister)
		m_AnimatedComps.AddAndReplace(c);
	else
		m_AnimatedComps.Erase(c);
}
void Scene::RegisterLight(Component* c, bool doRegister)
{
	if(doRegister)
		renderData->m_LightComps.AddAndReplace(c);
	else
		renderData->m_LightComps.Erase(c);
}
void Scene::RegisterFog(Component* c, bool doRegister)
{
	if(doRegister)
		renderData->m_FogComps.AddAndReplace(c);
	else
		renderData->m_FogComps.Erase(c);
}
void Scene::RegisterCamera(Component* c, bool doRegister)
{
	if(doRegister)
		renderData->m_CamComps.AddAndReplace(c);
	else
		renderData->m_CamComps.Erase(c);
}

void Scene::SetPassSettings(ERenderPass pass, const SceneRendererPassSettings& settings)
{
	renderData->m_PassSettings[pass] = settings;
}

const SceneRendererPassSettings& Scene::GetPassSettings(ERenderPass pass)
{
	return renderData->m_PassSettings[pass];
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto comp : m_AnimatedComps)
		comp->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

static void VisitComponentsRec(
	Node* node, ComponentVisitor* visitor)
{
	for(auto c : node->Components())
		visitor->Visit(c);
	if(visitor->ShouldAbortChildren()) {
		visitor->ResumeChildren();
		return;
	}
	for(auto child : node->Children())
		VisitComponentsRec(child, visitor);
}

void Scene::VisitComponents(
	ComponentVisitor* visitor,
	Node* root)
{
	if(!root)
		root = m_Root;
	VisitComponentsRec(root, visitor);
}

void Scene::DrawScene(bool beginScene, bool endScene)
{
	renderData->DrawScene(beginScene, endScene);
}

void InternalRenderData::DrawScene(bool beginScene, bool endScene)
{
	m_Renderer->SetParams(m_RendererAttributes);

	video::RenderStatistics::GroupScope grpScope("scene");

	// Collect visible camera nodes.
	m_Cameras.Clear();
	for(auto c : m_CamComps) {
		auto c2 = dynamic_cast<AbstractCamera*>(c);
		if(c2 && c2->GetNode()->IsTrulyVisible())
			m_Cameras.PushBack(c2);
	}

	// Collect lights and fogs.
	auto& lights = m_LightComps;
	auto& fogs = m_FogComps;

	m_Cameras.Sort(core::CompareTypeFromSmaller<AbstractCamera*>([](AbstractCamera* a, AbstractCamera* b) -> bool { return a->GetRenderPriority() > b->GetRenderPriority(); }));
	if(m_Cameras.Size() == 0) {
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
		m_ActiveCameraNode = m_ActiveCamera->GetNode();
		m_AbsoluteCamPos = m_ActiveCameraNode->GetAbsolutePosition();
		m_ActiveFrustum = m_ActiveCamera->GetFrustum();

		scene::SceneRenderData sceneData;
		sceneData.video = m_Renderer;
		sceneData.pass = ERenderPass::None;
		m_ActiveCamera->PreRender(sceneData);

		// Select visible lights and fogs.
		m_Lights.Clear();
		for(auto l : lights) {
			auto l2 = dynamic_cast<Light*>(l);
			if(l2 && l->GetNode()->IsTrulyVisible())
				m_Lights.PushBack(l2);
		}
		m_Fogs.Clear();
		for(auto f : fogs) {
			auto f2 = dynamic_cast<Fog*>(f);
			if(f2 && f->GetNode()->IsTrulyVisible())
				m_Fogs.PushBack(f2);
		}

		// Collect and renderable nodes.
		RenderableCollector collector(this);
		m_Scene->VisitComponents(&collector);

		// Sort by distance
		m_TransparentNodeList.Sort();

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

		m_ActiveCamera->Render(sceneData);
		DrawScene();
		m_ActiveCamera->PostRender(sceneData);

		if((it + 1) != m_Cameras.end() || endScene)
			m_Renderer->EndScene();

		m_SkyBoxList.Clear();
		m_SolidNodeList.Clear();
		m_ShadowCasters.Clear();
		m_TransparentNodeList.Clear();
		m_Lights.Clear();
		m_Fogs.Clear();
	}
}

void InternalRenderData::EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
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

void InternalRenderData::DisableOverwrite(video::PipelineOverwriteToken& token)
{
	if(m_SettingsActive) {
		m_Renderer->PopPipelineOverwrite(&token);
		m_SettingsActive = false;
	}
}

void InternalRenderData::AddRenderEntry(Node* n, Component* r)
{
	bool isCulled = false;

	switch(r->GetRenderPass()) {
	case ERenderPass::None:
		break;
	case ERenderPass::SkyBox:
		m_SkyBoxList.EmplaceBack(r);
		break;
	case ERenderPass::Solid:
		isCulled = IsCulled(n, r, m_ActiveFrustum);
		if(!isCulled)
			m_SolidNodeList.EmplaceBack(r);
		if(n->IsShadowCasting())
			m_ShadowCasters.EmplaceBack(r);
		break;
	case ERenderPass::Transparent:
		isCulled = IsCulled(n, r, m_ActiveFrustum);
		if(!isCulled) {
			float distance = n->GetAbsolutePosition().GetDistanceToSq(m_AbsoluteCamPos);
			m_TransparentNodeList.EmplaceBack(r, distance);
		}
		break;
	case ERenderPass::Any:
		isCulled = IsCulled(n, r, m_ActiveFrustum);
		if(!isCulled) {
			float distance = n->GetAbsolutePosition().GetDistanceToSq(m_AbsoluteCamPos);
			m_SolidNodeList.EmplaceBack(r);
			m_TransparentNodeList.EmplaceBack(r, distance);
		}
		if(n->IsShadowCasting())
			m_ShadowCasters.EmplaceBack(r);
		break;
	default:
		lxAssertNeverReach("Unimplemented render pass");
		break;
	}
}

bool InternalRenderData::IsCulled(Node* node, Component* r, const math::ViewFrustum& frustum)
{
	LUX_UNUSED(r);
	if(!m_Culling)
		return false;
	if(node->GetBoundingBox().IsEmpty())
		return false;
	return !math::IsOrientedBoxMaybeVisible(frustum, node->GetBoundingBox(), node->GetAbsoluteTransform());
}

static void SetFogData(video::Renderer* renderer, ClassicalFogDescription* desc, video::ColorF* overwriteColor = nullptr)
{
	if(desc) {
		video::ColorF fogB;
		video::ColorF fogA;
		fogA = overwriteColor ? *overwriteColor : desc->GetColor();
		renderer->GetParams().SetValue("fogA", fogA);
		auto type = desc->GetType();
		fogB.r =
			type == EFogType::Linear ? 1.0f :
			type == EFogType::Exp ? 2.0f :
			type == EFogType::ExpSq ? 3.0f : 0.0f;
		fogB.g = desc->GetStart();
		fogB.b = desc->GetEnd();
		fogB.a = desc->GetDensity();
		renderer->GetParams().SetValue("fogB", fogB);
	} else {
		video::ColorF fogB;
		fogB.r = 0.0f;
		renderer->GetParams().SetValue("fogB", fogB);
	}
}

static float LightTypeToFloat(scene::ELightType type)
{
	switch(type) {
	case scene::ELightType::Directional: return 1;
	case scene::ELightType::Point: return 2;
	case scene::ELightType::Spot: return 3;
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

void InternalRenderData::ClearLightData(video::Renderer* renderer)
{
	m_CurLightId = 0;
	renderer->GetParams().SetValue("light0", GenerateLightMatrix(nullptr));
	renderer->GetParams().SetValue("light1", GenerateLightMatrix(nullptr));
	renderer->GetParams().SetValue("light2", GenerateLightMatrix(nullptr));
	renderer->GetParams().SetValue("light3", GenerateLightMatrix(nullptr));
}

void InternalRenderData::AddLightData(video::Renderer* renderer, ClassicalLightDescription* desc)
{
	if(m_CurLightId < m_MaxLightsPerDraw) {
		core::String name;
		format::format(name, &format::InvariantLocale, "light{}", m_CurLightId);
		renderer->GetParams().SetValue(name, GenerateLightMatrix(desc));
	}
	++m_CurLightId;
}

void InternalRenderData::DrawScene()
{
	// Check if a stencil buffer is available for shadow rendering
	bool drawStencilShadows = m_SceneAttributes->GetValue<bool>("drawStencilShadows");
	if(drawStencilShadows) {
		// TODO: Should directly check the backbuffer.
		if(m_Renderer->GetDriver()->GetConfig().zsFormat.sBits == 0) {
			log::Warning("Scene: Can't draw stencil shadows without stencilbuffer(Disabled shadow rendering).");
			drawStencilShadows = false;
			m_SceneAttributes->SetValue("drawStencilShadows", false);
		}
	}

	m_Culling = m_SceneAttributes->GetValue<bool>("culling");

	struct LightEntry
	{
		ClassicalLightDescription* desc;
		Node* node;
	};
	core::Array<LightEntry> illuminating;
	core::Array<LightEntry> shadowCasting;
	core::Array<LightEntry> nonShadowCasting;
	SceneRenderData sceneData;
	sceneData.video = m_Renderer;
	sceneData.activeCamera = m_ActiveCamera;

	m_Renderer->GetParams().SetValue("camPos", m_ActiveCameraNode->GetAbsolutePosition());

	//-------------------------------------------------------------------------
	// The lights
	ClearLightData(m_Renderer);
	video::ColorF totalAmbientLight;
	int maxShadowCastingCount = m_SceneAttributes->GetValue<int>("maxShadowCasters");
	if(!drawStencilShadows)
		maxShadowCastingCount = 0;
	maxShadowCastingCount = math::Clamp(maxShadowCastingCount, 0, m_MaxLightsPerDraw);

	int count = 0;
	int shadowCount = 0;
	for(auto& e : m_Lights) {
		auto descBase = e->GetLightDescription();
		if(auto descC = dynamic_cast<ClassicalLightDescription*>(descBase)) {
			LightEntry entry{descC, e->GetNode()};
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
	m_Renderer->GetParams().SetValue("ambient", totalAmbientLight);

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
		e.renderable->Render(sceneData);

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
	for(auto& e : m_SolidNodeList)
		e.renderable->Render(sceneData);
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
			for(auto& e : m_ShadowCasters) {
				auto mesh = dynamic_cast<Mesh*>(e.renderable);
				if(mesh && mesh->GetMesh()) {
					bool isInfinite;
					math::Vector3F lightPos;
					auto node = e.renderable->GetNode();
					if(shadowLight->GetType() == scene::ELightType::Directional) {
						isInfinite = true;
						lightPos = node->ToRelativeDir(shadowNode->FromRelativeDir(math::Vector3F::UNIT_Z));
					} else {
						isInfinite = false;
						lightPos = node->ToRelativePos(shadowNode->GetAbsolutePosition());
					}
					m_StencilShadowRenderer.AddSilhouette(node->GetAbsoluteTransform(), mesh->GetMesh(), lightPos, isInfinite);
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

			for(auto& e : m_SolidNodeList)
				e.renderable->Render(sceneData);

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
			for(auto& e : m_SolidNodeList)
				e.renderable->Render(sceneData);

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

	for(auto& e : m_TransparentNodeList) {
		e.renderable->Render(sceneData);
	}

	DisableOverwrite(pot); // Transparent
}

} // namespace scene
} // namespace lux