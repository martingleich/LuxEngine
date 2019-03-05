#include "scene/Scene.h"

#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"

#include "core/Logger.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "scene/components/Fog.h"
#include "scene/components/Light.h"
#include "scene/components/SceneMesh.h"

#include "scene/StencilShadowRenderer.h"

namespace lux
{
namespace scene
{

// Helper functions.
namespace
{
template <typename T>
class VisibleComponentCache
{
public:
	template <typename RangeT>
	void Update(const RangeT& range)
	{
		m_Comps.Clear();
		for(auto& c : range) {
			auto c2 = dynamic_cast<T>(c);
			if(c2 && c2->GetNode()->IsTrulyVisible())
				m_Comps.PushBack(c2);
		}
	}

	auto begin() { return m_Comps.begin(); }
	auto end() { return m_Comps.end(); }

	core::Array<T>& AsArray() const { return m_Comps; }
private:
	core::Array<T> m_Comps;
};

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

void VisitComponentsRec(
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

class RenderableCollector : public ComponentVisitor
{
public:
	void Update(
		const math::Vector3F& camPos,
		Node* root)
	{
		m_Culling = false;
		m_CamPos = camPos;
		Clear();

		VisitComponentsRec(root, this);
		transparentNodeList.Sort();
	}

	void Update(
		const math::ViewFrustum& visibleFrustum,
		const math::Vector3F& camPos,
		Node* root)
	{
		m_Culling = true;
		m_Frustum = visibleFrustum;
		m_CamPos = camPos;

		Clear();
		VisitComponentsRec(root, this);
		transparentNodeList.Sort();
	}

	void Visit(Component* c)
	{
		if(c->GetNode()->IsTrulyVisible())
			AddRenderEntry(c->GetNode(), c);
	}

	core::Array<RenderEntry> skyBoxList;
	core::Array<RenderEntry> solidNodeList;
	core::Array<RenderEntry> shadowCasters;
	core::Array<DistanceRenderEntry> transparentNodeList;

private:
	void AddRenderEntry(Node* n, Component* r)
	{
		bool isCulled = IsCulled(n, r, m_Frustum);

		for(ERenderPass pass : r->GetRenderPass()) {
			switch(pass) {
			case ERenderPass::SkyBox:
				skyBoxList.EmplaceBack(r);
				break;
			case ERenderPass::Solid:
				if(!isCulled)
					solidNodeList.EmplaceBack(r);
				if(n->IsShadowCasting())
					shadowCasters.EmplaceBack(r);
				break;
			case ERenderPass::Transparent:
				if(!isCulled) {
					float distance = n->GetAbsolutePosition().GetDistanceToSq(m_CamPos);
					transparentNodeList.EmplaceBack(r, distance);
				}
				break;
			default:
				lxAssertNeverReach("Unimplemented render pass");
				break;
			}
		}
	}

	bool IsCulled(Node* node, Component* r, const math::ViewFrustum& frustum)
	{
		LUX_UNUSED(r);
		if(!m_Culling)
			return false;
		if(node->GetBoundingBox().IsEmpty())
			return false;
		return !math::IsOrientedBoxMaybeVisible(frustum, node->GetBoundingBox(), node->GetAbsoluteTransform());
	}

	void Clear()
	{
		skyBoxList.Clear();
		solidNodeList.Clear();
		shadowCasters.Clear();
		transparentNodeList.Clear();
	}
private:
	math::ViewFrustum m_Frustum;
	math::Vector3F m_CamPos;
	bool m_Culling;
};

void SetFogData(video::Renderer* renderer, ClassicalFogDescription* desc, video::ColorF* overwriteColor = nullptr)
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

float LightTypeToFloat(scene::ELightType type)
{
	switch(type) {
	case scene::ELightType::Directional: return 1;
	case scene::ELightType::Point: return 2;
	case scene::ELightType::Spot: return 3;
	default:
		throw core::GenericInvalidArgumentException("type", "Unknown light data type");
	}
}

math::Matrix4 GenerateLightMatrix(ClassicalLightDescription* desc)
{
	/*
	Lux illumination matrix.
	Only the location of the t value is fixed, all other values depend on the light
	type, for built-in point/directonal and spot light the matrix is build as
	following:

	 r  g  b t
	px py pz 0
	dx dy dz 0
	fa ic oc 0

	t = Type of light (0 = Disabled, 1 = Directional, 2 = Point, 3 = Spot)

	(r,g,b) = Diffuse color of the light
	(px,py,pz) = Position of the light
	(dx,dy,dz) = Direction of the light

	fa = Falloff for the spotlight
	ic = Cosine of half inner cone for the spotlight
	oc = Cosine of half outer cone for the spotlight
	*/

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

class LightDataManager
{
public:
	LightDataManager(video::Renderer* renderer, int maxLightsPerDraw) :
		m_MaxLightsPerDraw(maxLightsPerDraw),
		m_Renderer(renderer)
	{
	}

	void Reset()
	{
		m_CurLightId = 0;
		for(int i = 0; i < m_MaxLightsPerDraw; ++i)
			SetLight(i, nullptr);
	}

	void AddLight(ClassicalLightDescription* desc)
	{
		if(m_CurLightId < m_MaxLightsPerDraw)
			SetLight(m_CurLightId, desc);
		++m_CurLightId;
	}

	int GetMaxLightsPerDraw() const
	{
		return m_MaxLightsPerDraw;
	}
private:
	core::String GetLightName(int id)
	{
		core::String name;
		format::format(name, &format::InvariantLocale, "light{}", id);
		return name;
	}

	void SetLight(int id, ClassicalLightDescription* desc)
	{
		auto name = GetLightName(id);
		auto mat = GenerateLightMatrix(desc);
		m_Renderer->GetParams().SetValue(name, mat);
	}

private:
	video::Renderer* m_Renderer;
	int m_CurLightId;
	const int m_MaxLightsPerDraw;
};

class ScenePassManager
{
public:
	ScenePassManager(video::Renderer* r, Scene* s) :
		m_Renderer(r),
		m_Scene(s)
	{
	}
	void StartPass(ERenderPass pass)
	{
		if(m_CurPass != pass)
			EndPass(m_CurPass);
		m_CurPass = pass;

		auto& debugSettings = m_Scene->GetDebugSettings();
		if(pass == ERenderPass::Solid || pass == ERenderPass::Transparent) {
			if(debugSettings.renderWireframe) {
				video::PipelineOverwrite overwrite;
				if(debugSettings.renderWireframe) {
					overwrite.OverwriteDrawMode(video::EDrawMode::Wire);
					m_Renderer->PushPipelineOverwrite(overwrite, &m_Token);
				}
			}
		}
	}

private:
	void EndPass(ERenderPass pass)
	{
		LUX_UNUSED(pass);
		if(m_Token.count)
			m_Renderer->PopPipelineOverwrite(&m_Token);
	}
private:
	video::Renderer* m_Renderer;
	ERenderPass m_CurPass = ERenderPass::Unknown;
	Scene* m_Scene;
	video::PipelineOverwriteToken m_Token;
};

}

class InternalRenderData : public SceneRenderPassHelper
{
public:
	InternalRenderData(Scene* scene, video::Renderer* renderer, core::AttributeList* attributes) :
		m_Scene(scene),
		m_Renderer(renderer),
		m_SceneAttributes(attributes),
		m_VideoLights(renderer, 4),
		m_StencilShadowRenderer(renderer, 0xFFFFFFFF)
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

	void DrawScene()
	{
		if(m_PassControllers.IsEmpty())
			log::Warning("No renderobject in the scenegraph.");

		// Sort visible cameras by render id
		core::Array<SceneRenderPassController*> sortedPassControllers;
		for(auto c : m_PassControllers)
			sortedPassControllers.PushBack(c);
		core::Sort(sortedPassControllers,
			core::CompareTypeFromSmaller<SceneRenderPassController*>([](SceneRenderPassController* a, SceneRenderPassController* b) -> bool {
			return a->GetPriority() > b->GetPriority();
		}));

		// Backup and update video renderer scene argument pool.
		auto oldParams = m_Renderer->GetParams();
		m_Renderer->SetParams(m_RendererAttributes);

		// Render each pass.
		m_CurPassId = 0;
		m_PassCount = sortedPassControllers.Size();
		for(auto c : sortedPassControllers) {
			c->Render(this);
			++m_CurPassId;
		}

		// Restore old scene argument pool.
		m_Renderer->SetParams(oldParams);
	}

	// Update and repair broken configurations
	void UpdateConfiguration()
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
	}

	core::Optional<ClassicalFogDescription*> ComputeSingleClassicalFog()
	{
		core::Optional<ClassicalFogDescription*> fog;
		bool hasReportError = false;
		for(auto& f : m_VisibleFogs) {
			auto fogDesc = dynamic_cast<ClassicalFogDescription*>(f->GetFogDescription());
			if(fogDesc) {
				if(!fog.HasValue())
					fog = fogDesc;
				else if(!hasReportError) {
					log::Warning("Simple Forward Scene Renderer: Only support one fog element per scene.(all but one fog disabled)");
					hasReportError = true;
				}
			}
		}
		return fog;
	}

	void ComputeClassicalLights(core::Array<ClassicalLightDescription*>& lights, video::ColorF& ambientLight)
	{
		ambientLight = video::ColorF(0,0,0,0);
		lights.Clear();

		int count = 0;
		for(auto& e : m_VisibleLights) {
			auto descBase = e->GetLightDescription();
			if(auto descC = dynamic_cast<ClassicalLightDescription*>(descBase)) {
				lights.PushBack(descC);
				++count;
				if(count >= m_VideoLights.GetMaxLightsPerDraw())
					break;
			} else if(auto descA = dynamic_cast<AmbientLightDescription*>(descBase)) {
				ambientLight += descA->GetColor();
			}
		}
	}

	void ComputeClassicalShadowingLights(
		int maxIlluminating,
		int maxShadowCasters,
		core::Array<ClassicalLightDescription*> illuminating,
		core::Array<ClassicalLightDescription*> shadowCasting,
		core::Array<ClassicalLightDescription*> nonShadowCasting,
		video::ColorF& ambientLight)
	{
		illuminating.Clear();
		shadowCasting.Clear();
		nonShadowCasting.Clear();
		ambientLight = video::ColorF(0,0,0,0);

		int count = 0;
		int shadowCount = 0;
		for(auto& e : m_VisibleLights) {
			auto descBase = e->GetLightDescription();
			if(auto descC = dynamic_cast<ClassicalLightDescription*>(descBase)) {
				illuminating.PushBack(descC);
				if(descC->IsShadowCasting() && shadowCount < maxShadowCasters) {
					shadowCasting.PushBack(descC);
					++shadowCount;
				} else {
					nonShadowCasting.PushBack(descC);
				}
				++count;
				if(count >= maxIlluminating)
					break;
			} else if(auto descA = dynamic_cast<AmbientLightDescription*>(descBase)) {
				ambientLight += descA->GetColor();
			}
		}
	}

	void DrawScenePass_NoShadow(const SceneRenderCamData& camData)
	{
		SceneRenderData sceneData;
		sceneData.video = m_Renderer;
		sceneData.camData = camData;

		m_Renderer->GetParams().SetValue("camPos", camData.transform.translation);

		//-------------------------------------------------------------------------
		// The lights
		core::Array<ClassicalLightDescription*> illuminating;
		video::ColorF totalAmbientLight;
		ComputeClassicalLights(illuminating, totalAmbientLight);

		// Enable lights
		m_Renderer->GetParams().SetValue("ambient", totalAmbientLight);
		m_VideoLights.Reset();
		for(auto light : illuminating)
			m_VideoLights.AddLight(light);

		//-------------------------------------------------------------------------
		// The fog
		auto fog = ComputeSingleClassicalFog();
		if(fog.HasValue())
			SetFogData(m_Renderer, fog.GetValue());

		// Real object rendering starts here.
		ScenePassManager passManager(m_Renderer, m_Scene);

		//-------------------------------------------------------------------------
		// Skyboxes
		passManager.StartPass(ERenderPass::SkyBox);
		sceneData.pass = ERenderPass::SkyBox;
		for(auto& e : m_RenderableCollection.skyBoxList)
			e.renderable->Render(sceneData);

		//-------------------------------------------------------------------------
		// Solid objects
		passManager.StartPass(ERenderPass::Solid);
		sceneData.pass = ERenderPass::Solid;
		for(auto& e : m_RenderableCollection.solidNodeList)
			e.renderable->Render(sceneData);

		//-------------------------------------------------------------------------
		// Transparent objects
		passManager.StartPass(ERenderPass::Transparent);
		sceneData.pass = ERenderPass::Transparent;
		for(auto& e : m_RenderableCollection.transparentNodeList)
			e.renderable->Render(sceneData);
	}

	void DrawScenePass_StencilShadows(const SceneRenderCamData& camData)
	{
		SceneRenderData sceneData;
		sceneData.video = m_Renderer;
		sceneData.camData = camData;

		m_Renderer->GetParams().SetValue("camPos", camData.transform.translation);

		//-------------------------------------------------------------------------
		// The lights
		m_VideoLights.Reset();

		int maxShadowCastingCount = m_SceneAttributes->GetValue<int>("maxShadowCasters");
		maxShadowCastingCount = math::Clamp(maxShadowCastingCount, 0, m_VideoLights.GetMaxLightsPerDraw());

		core::Array<ClassicalLightDescription*> illuminating;
		core::Array<ClassicalLightDescription*> shadowCasting;
		core::Array<ClassicalLightDescription*> nonShadowCasting;
		video::ColorF ambientLight;
		ComputeClassicalShadowingLights(
			m_VideoLights.GetMaxLightsPerDraw(),
			maxShadowCastingCount,
			illuminating, shadowCasting, nonShadowCasting,
			ambientLight);

		// Enable ambient light.
		m_Renderer->GetParams().SetValue("ambient", ambientLight);

		//-------------------------------------------------------------------------
		// The fog
		core::Optional<ClassicalFogDescription*> fog = ComputeSingleClassicalFog();
		if(fog.HasValue())
			SetFogData(m_Renderer, fog.GetValue());

		// Real object rendering starts here.
		ScenePassManager passManager(m_Renderer, m_Scene);

		//-------------------------------------------------------------------------
		// Skyboxes
		passManager.StartPass(ERenderPass::SkyBox);
		sceneData.pass = ERenderPass::SkyBox;
		for(auto& e : m_RenderableCollection.skyBoxList)
			e.renderable->Render(sceneData);

		//-------------------------------------------------------------------------
		// Solid objects
		passManager.StartPass(ERenderPass::Solid);
		{
			video::PipelineOverwriteToken pot;
			video::PipelineOverwrite illumOver;
			illumOver.OverwriteLighting(video::ELightingFlag::AmbientEmit);
			m_Renderer->PushPipelineOverwrite(illumOver, &pot); // Push 1

			// Ambient pass
			sceneData.pass = ERenderPass::Solid;
			for(auto& e : m_RenderableCollection.solidNodeList)
				e.renderable->Render(sceneData);

			m_Renderer->PopPipelineOverwrite(&pot); // Pop 1
		}

		passManager.StartPass(ERenderPass::Unknown);
		const bool correctFogForStencilShadows = true;
		// To renderer correct fog, render black fog.
		if(correctFogForStencilShadows && fog.HasValue()) {
			auto overwriteColor = video::ColorF(0, 0, 0, 0);
			SetFogData(m_Renderer, fog.GetValue(), &overwriteColor);
		}

		// Shadow pass for each shadow casting light
		for(auto shadowLight : shadowCasting) {
			m_VideoLights.Reset();
			m_VideoLights.AddLight(shadowLight);

			m_StencilShadowRenderer.Begin(
				camData.transform.translation,
				camData.transform.TransformDir(math::Vector3F::UNIT_Z));
			for(auto& e : m_RenderableCollection.shadowCasters) {
				auto mesh = dynamic_cast<Mesh*>(e.renderable);
				if(mesh && mesh->GetMesh()) {
					bool isInfinite;
					math::Vector3F lightPos;
					auto node = e.renderable->GetNode();
					if(shadowLight->GetType() == scene::ELightType::Directional) {
						isInfinite = true;
						lightPos = node->ToRelativeDir(shadowLight->GetDirection());
					} else {
						isInfinite = false;
						lightPos = node->ToRelativePos(shadowLight->GetPosition());
					}
					m_StencilShadowRenderer.AddSilhouette(node->GetAbsoluteTransform(), mesh->GetMesh(), lightPos, isInfinite);
				}
			}

			m_StencilShadowRenderer.End();

			passManager.StartPass(ERenderPass::Solid);
			{
				video::PipelineOverwriteToken pot;
				video::PipelineOverwrite illumOver;
				illumOver.OverwriteZWrite(false);
				illumOver.OverwriteLighting(video::ELightingFlag::DiffSpec);
				illumOver.OverwriteAlpha(video::AlphaBlendMode(video::EBlendFactor::One, video::EBlendFactor::One, video::EBlendOperator::Add));
				illumOver.OverwriteStencil(m_StencilShadowRenderer.GetIllumniatedStencilMode());
				m_Renderer->PushPipelineOverwrite(illumOver, &pot); // Push 2

				for(auto& e : m_RenderableCollection.solidNodeList)
					e.renderable->Render(sceneData);

				m_Renderer->PopPipelineOverwrite(&pot); // Pop 2
			}

			m_Renderer->Clear(false, false, true);
		}

		if(!nonShadowCasting.IsEmpty()) {
			m_VideoLights.Reset();
			// Draw with remaining non shadow casting lights
			for(auto illum : nonShadowCasting)
				m_VideoLights.AddLight(illum);

			{
				video::PipelineOverwriteToken pot;
				video::PipelineOverwrite illumOver;
				illumOver.OverwriteZWrite(false);
				illumOver.OverwriteLighting(video::ELightingFlag::DiffSpec);
				illumOver.OverwriteAlpha(video::AlphaBlendMode(video::EBlendFactor::One, video::EBlendFactor::One, video::EBlendOperator::Add));
				m_Renderer->PushPipelineOverwrite(illumOver, &pot); // Push 3

				sceneData.pass = ERenderPass::Solid;
				for(auto& e : m_RenderableCollection.solidNodeList)
					e.renderable->Render(sceneData);

				m_Renderer->PopPipelineOverwrite(&pot); // Pop 3
			}
		}

		// Add shadow casting lights for remaining render jobs
		for(auto light : shadowCasting)
			m_VideoLights.AddLight(light);

		// Restore correct fog
		if(correctFogForStencilShadows && fog.HasValue())
			SetFogData(m_Renderer, fog.GetValue());

		//-------------------------------------------------------------------------
		// Transparent objects
		passManager.StartPass(ERenderPass::Transparent);
		sceneData.pass = ERenderPass::Transparent;
		for(auto& e : m_RenderableCollection.transparentNodeList)
			e.renderable->Render(sceneData);
	}

	void DrawScenePass(const SceneRenderCamData& camData)
	{

		UpdateConfiguration();

		bool drawStencilShadows = m_SceneAttributes->GetValue<bool>("drawStencilShadows");

		if(drawStencilShadows)
			DrawScenePass_StencilShadows(camData);
		else
			DrawScenePass_NoShadow(camData);
	}

	// Scene Render Pass Helper functions.
	video::Renderer* GetRenderer() override { return m_Renderer; }

	void DefaultRenderScene(const SceneRenderPassDefaultData& data)
	{
		auto& camData = data.camData;

		// Setup scene
		// Select visible lights and fogs.
		m_VisibleLights.Update(m_LightComps);
		m_VisibleFogs.Update(m_FogComps);

		// Collect all renderable nodes.
		if(m_SceneAttributes->GetValue<bool>("culling")) {
			m_RenderableCollection.Update(
				camData.frustum, camData.transform.translation, m_Scene->GetRoot());
		} else {
			m_RenderableCollection.Update(
				camData.transform.translation, m_Scene->GetRoot());
		}

		// Begin scene
		m_Renderer->SetRenderTarget(data.renderTarget);
		m_Renderer->Clear(false, true, true);
		// Color clear is performed by skybox.
		m_Renderer->BeginScene();

		data.onPreRender.Call();

		// Render scene.
		DrawScenePass(camData);

		data.onPostRender.Call();

		// End scene
		if(m_CurPassId + 1 < m_PassCount)
			m_Renderer->EndScene();
	}

public:
	// Directly written by the Scene.
	core::HashSet<Component*> m_LightComps; //!< The animated nodes of the graph
	core::HashSet<Component*> m_FogComps; //!< The animated nodes of the graph
	core::HashSet<SceneRenderPassController*> m_PassControllers; //!< The animated nodes of the graph

private:
	Scene* m_Scene;
	video::Renderer* m_Renderer;
	core::AttributeList m_RendererAttributes;

	core::AttributeList* m_SceneAttributes;

	int m_CurPassId;
	int m_PassCount;

	RenderableCollector m_RenderableCollection;
	VisibleComponentCache<Light*> m_VisibleLights;
	VisibleComponentCache<Fog*> m_VisibleFogs;

	LightDataManager m_VideoLights;
	StencilShadowRenderer m_StencilShadowRenderer;
};

////////////////////////////////////////////////////////////////////////////////////

Scene::Scene() :
	m_Root(LUX_NEW(Node)(this)),
	renderData(new InternalRenderData(this, video::VideoDriver::Instance()->GetRenderer(), &m_Attributes))
{
	core::AttributeListBuilder alb;
	alb.AddAttribute("drawStencilShadows", false);
	alb.AddAttribute("maxShadowCasters", 1);
	alb.AddAttribute("culling", true);
	m_Attributes = alb.BuildAndReset();
}

Scene::~Scene()
{
	// Explicitly free all nodes
	m_Root = nullptr;
	ClearDeletionQueue();
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
void Scene::RegisterRenderController(SceneRenderPassController* c, bool doRegister)
{
	if(doRegister)
		renderData->m_PassControllers.AddAndReplace(c);
	else
		renderData->m_PassControllers.Erase(c);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto comp : m_AnimatedComps)
		comp->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::VisitComponents(
	ComponentVisitor* visitor,
	Node* root)
{
	if(!root)
		root = m_Root;
	VisitComponentsRec(root, visitor);
}

void Scene::DrawScene()
{
	video::RenderStatistics::GroupScope grpScope("scene");

	renderData->DrawScene();

	ClearDeletionQueue();
}

} // namespace scene
} // namespace lux