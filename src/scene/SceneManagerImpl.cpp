#include "scene/SceneManagerImpl.h"

#include "video/DriverConfig.h"
#include "video/VideoDriver.h"
#include "video/RenderTarget.h"

#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"
#include "video/mesh/MeshSystem.h"
#include "video/mesh/VideoMesh.h"

#include "resources/ResourceSystem.h"
#include "core/ReferableFactory.h"

#include "scene/components/Camera.h"
#include "scene/components/SceneMesh.h"
#include "scene/components/Light.h"
#include "scene/components/SkyBox.h"

#include "scene/components/RotationAnimator.h"
#include "scene/components/LinearMoveAnimator.h"
#include "scene/components/CameraControl.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"

#include "scene/collider/SphereCollider.h"
#include "scene/collider/BoxCollider.h"
#include "scene/collider/MeshCollider.h"

#include "core/Logger.h"

#include "video/VertexTypes.h"

namespace lux
{
namespace scene
{

SceneManagerImpl::SceneManagerImpl() :
	m_CollectedRoot(nullptr),
	m_StencilShadowRenderer(video::VideoDriver::Instance()->GetRenderer(), 0xFFFFFFFF)
{
	m_RootSceneNode = LUX_NEW(Node)(this, true);
	m_Renderer = video::VideoDriver::Instance()->GetRenderer();

	m_Fog.isActive = false;

	m_Attributes.AddAttribute("drawStencilShadows", false);
	m_Attributes.AddAttribute("maxShadowCasters", 1);
}

SceneManagerImpl::~SceneManagerImpl()
{
	Clear();
}

void SceneManagerImpl::Clear()
{
	ClearDeletionQueue();

	m_CameraList.Clear();
	m_LightList.Clear();

	m_RootSceneNode->RemoveAllChildren();
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Node> SceneManagerImpl::AddNode(Component* baseComp, Node* parent)
{
	if(!parent)
		parent = GetRoot();

	auto node = parent->AddChild();
	if(baseComp)
		node->AddComponent(baseComp);

	return node;
}

StrongRef<Node> SceneManagerImpl::AddMesh(const io::Path& path)
{
	return AddNode(CreateMesh(path));
}

StrongRef<Node> SceneManagerImpl::AddMesh(video::Mesh* mesh)
{
	return AddNode(CreateMesh(mesh));
}

StrongRef<Node> SceneManagerImpl::AddSkyBox(const video::Colorf& color)
{
	auto box = CreateSkyBox();
	box->GetMaterial(0)->SetDiffuse(color);
	return AddNode(box);
}

StrongRef<Node> SceneManagerImpl::AddSkyBox(video::CubeTexture* skyTexture)
{
	return AddNode(CreateSkyBox(skyTexture));
}

StrongRef<Node> SceneManagerImpl::AddLight(video::ELightType lightType, video::Color color)
{
	return AddNode(CreateLight(lightType, color));
}

StrongRef<Node> SceneManagerImpl::AddCamera()
{
	return AddNode(CreateCamera());
}

StrongRef<Camera> SceneManagerImpl::CreateCamera()
{
	return CreateComponent(SceneComponentType::Camera);
}

StrongRef<Mesh> SceneManagerImpl::CreateMesh(const io::Path& path)
{
	return CreateMesh(core::ResourceSystem::Instance()->GetResource(core::ResourceType::Mesh, path).As<video::Mesh>());
}

StrongRef<Mesh> SceneManagerImpl::CreateMesh(video::Mesh* mesh)
{
	StrongRef<Mesh> out = CreateComponent(SceneComponentType::Mesh);
	out->SetMesh(mesh);

	return out;
}

StrongRef<SkyBox> SceneManagerImpl::CreateSkyBox(video::CubeTexture* skyTexture)
{
	StrongRef<SkyBox> out = CreateComponent(SceneComponentType::SkyBox);
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<Light> SceneManagerImpl::CreateLight(video::ELightType lightType, video::Color color)
{
	StrongRef<Light> light = CreateComponent(SceneComponentType::Light);
	light->SetLightType(lightType);
	light->SetColor(color);
	return light;
}

StrongRef<RotationAnimator> SceneManagerImpl::CreateRotator(const math::Vector3F& axis, math::AngleF rotSpeed)
{
	StrongRef<RotationAnimator> out = CreateComponent(SceneComponentType::Rotation);
	out->SetAxis(axis);
	out->SetRotationSpeed(rotSpeed);

	return out;
}

StrongRef<LinearMoveAnimator> SceneManagerImpl::CreateLinearMover(const math::Line3F& line, float duration)
{
	StrongRef<LinearMoveAnimator> out = CreateComponent(SceneComponentType::LinearMove);
	out->SetData(line, duration);

	return out;
}

StrongRef<CameraControl> SceneManagerImpl::CreateCameraControl(float moveSpeed, math::AngleF rotSpeed, bool noVerticalMovement)
{
	StrongRef<CameraControl> out = CreateComponent(SceneComponentType::CameraControl);
	out->SetMoveSpeed(moveSpeed);
	out->SetRotationSpeed(rotSpeed);
	out->AllowVerticalMovement(!noVerticalMovement);

	return out;
}

StrongRef<Component> SceneManagerImpl::CreateComponent(core::Name type)
{
	return core::ReferableFactory::Instance()->Create(type);
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Collider> SceneManagerImpl::CreateMeshCollider(video::Mesh* mesh)
{
	return LUX_NEW(MeshCollider)(mesh);
}

StrongRef<Collider> SceneManagerImpl::CreateBoundingBoxCollider()
{
	return LUX_NEW(BoundingBoxCollider)();
}

StrongRef<Collider> SceneManagerImpl::CreateBoundingSphereCollider()
{
	return LUX_NEW(BoundingSphereCollider)();
}

StrongRef<Collider> SceneManagerImpl::CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans)
{
	return LUX_NEW(BoxCollider)(halfSize, trans);
}

StrongRef<Collider> SceneManagerImpl::CreateSphereCollider(const math::Vector3F& center, float radius)
{
	return LUX_NEW(SphereCollider)(center, radius);
}

////////////////////////////////////////////////////////////////////////////////////

Node* SceneManagerImpl::GetRoot()
{
	return m_RootSceneNode;
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::AnimateAll(float secsPassed)
{
	for(auto node : m_AnimatedNodes) {
		node->Animate(secsPassed);
	}
}

bool SceneManagerImpl::DrawAll(bool beginScene, bool endScene)
{
	auto oldActiveCamNode = m_ActiveCameraNode;
	auto oldActiveCam = m_ActiveCamera;

	ClearDeletionQueue();

	// Collect "real" camera nodes
	auto camList = m_CameraList;
	if(camList.Size() == 0) {
		log::Warning("No camera in scenegraph.");
		return false;
	}

	auto newEnd = core::RemoveIf(camList,
		[](const CameraEntry& e) -> bool {
		return !e.node->IsTrulyVisible();
	});
	camList.Resize(core::IteratorDistance(camList.First(), newEnd));

	camList.Sort();

	if(camList.IsEmpty())
		throw core::ErrorException("Scene contains no visible camera.");

	if(!beginScene) {
		if(m_Renderer->GetRenderTarget() != camList[0].camera->GetRenderTarget())
			throw core::ErrorException("Already started scene uses diffrent rendertarget than first camera.");
	}

	for(auto it = camList.First(); it != camList.End(); ++it) {
		CollectRenderables(m_RootSceneNode);

		m_ActiveCamera = it->camera;
		m_ActiveCameraNode = it->node;
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

		m_ActiveCamera->PreRender(m_Renderer, m_ActiveCameraNode);

		m_ActiveCamera->Render(m_Renderer, m_ActiveCameraNode);

		DrawScene();

		m_ActiveCamera->PostRender(m_Renderer, m_ActiveCameraNode);

		if(it != camList.Last() || endScene)
			m_Renderer->EndScene();
	}

	m_SkyBoxList.Clear();
	m_SolidNodeList.Clear();
	m_TransparentNodeList.Clear();

	m_CollectedRoot = nullptr;

	m_ActiveCameraNode = oldActiveCamNode;
	m_ActiveCamera = oldActiveCam;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::RegisterCamera(Node* node, Camera* camera)
{
	m_CameraList.PushBack(CameraEntry(node, camera));
}

void SceneManagerImpl::UnregisterCamera(Node* node, Camera* camera)
{
	CameraEntry entry(node, camera);
	auto it = core::LinearSearch(entry, m_CameraList);
	if(it != m_CameraList.End())
		m_CameraList.Erase(it);
}

void SceneManagerImpl::RegisterLight(Node* node, Light* light)
{
	m_LightList.PushBack(LightEntry(node, light));
}

void SceneManagerImpl::UnregisterLight(Node* node, Light* light)
{
	LightEntry entry(node, light);
	auto it = core::LinearSearch(entry, m_LightList);
	if(it != m_LightList.End())
		m_LightList.Erase(it);
}

void SceneManagerImpl::RegisterAnimated(Node* node)
{
	m_AnimatedNodes.Insert(node);
}

void SceneManagerImpl::UnregisterAnimated(Node* node)
{
	m_AnimatedNodes.Erase(node);
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::AddToDeletionQueue(Node* node)
{
	if(node)
		m_DeletionQueue.PushBack(node);
}

void SceneManagerImpl::ClearDeletionQueue()
{
	for(auto it = m_DeletionQueue.First(); it != m_DeletionQueue.End(); ++it)
		(*it)->Remove();

	m_DeletionQueue.Clear();
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::SetAmbient(const video::Colorf& ambient)
{
	m_AmbientColor = ambient;
}

const video::Colorf& SceneManagerImpl::GetAmbient() const
{
	return m_AmbientColor;
}

void SceneManagerImpl::SetFog(const video::FogData& fog)
{
	m_Fog = fog;
}

const video::FogData& SceneManagerImpl::GetFog() const
{
	return m_Fog;
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::PushPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over)
{
	m_Overwrites[pass].PushBack(over);
}

void SceneManagerImpl::PopPipelineOverwrite(ERenderPass pass)
{
	m_Overwrites[pass].PopBack();
}

////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::EnableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	for(auto& over : m_Overwrites[pass])
		m_Renderer->PushPipelineOverwrite(over, &token);
}

void SceneManagerImpl::DisableOverwrite(ERenderPass pass, video::PipelineOverwriteToken& token)
{
	for(auto& over : m_Overwrites[pass]) {
		LUX_UNUSED(over);
		m_Renderer->PopPipelineOverwrite(&token);
	}
}

class SceneManagerImpl::RenderableCollector : public RenderableVisitor
{
public:
	void Visit(Renderable* r)
	{
		smgr->AddRenderEntry(curNode, r);
	}

	Node* curNode;
	SceneManagerImpl* smgr;
};

void SceneManagerImpl::CollectRenderables(Node* root)
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

void SceneManagerImpl::CollectRenderablesRec(Node* node, RenderableCollector* collector, bool noDebug)
{
	collector->curNode = node;
	node->VisitRenderables(collector, noDebug);

	for(auto child : node->Children())
		CollectRenderablesRec(child, collector, noDebug);
}

void SceneManagerImpl::AddRenderEntry(Node* n, Renderable* r)
{
	switch(r->GetRenderPass()) {
	case ERenderPass::SkyBox:
		m_SkyBoxList.PushBack(RenderEntry(n, r));
		break;
	case ERenderPass::Solid:
		m_SolidNodeList.PushBack(RenderEntry(n, r));
		break;
	case ERenderPass::Transparent:
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r));
		break;
	case ERenderPass::SolidAndTransparent:
		m_SolidNodeList.PushBack(RenderEntry(n, r));
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r));
		break;
	default:
		break;
	}
}

bool SceneManagerImpl::IsCulled(Node* n, Renderable* r)
{
	LUX_UNUSED(n);
	LUX_UNUSED(r);

	return false;
}

void SceneManagerImpl::DrawScene()
{
	m_Renderer->SetFog(m_Fog);

	// Check if a stencil buffer is available for shadow rendering
	bool drawStencilShadows = m_Attributes["drawStencilShadows"];
	if(drawStencilShadows) {
		if(m_Renderer->GetDriver()->GetConfig().zsFormat.sBits == 0) {
			log::Warning("Scene: Can't draw stencil shadows without stencilbuffer(Disabled shadow rendering).");
			drawStencilShadows = false;
			m_Attributes["drawStencilShadows"] = false;
		}
	}

	core::Array<SceneData::LightEntry> illuminating;
	core::Array<SceneData::LightEntry> shadowCasting;
	core::Array<SceneData::LightEntry> nonShadowCasting;
	SceneData sceneData(illuminating, shadowCasting);
	sceneData.activeCamera = m_ActiveCamera;
	sceneData.activeCameraNode = m_ActiveCameraNode;

	//-------------------------------------------------------------------------
	// The lights
	*m_Renderer->GetParam("ambient") = m_AmbientColor;

	m_Renderer->ClearLights();

	size_t maxLightCount = m_Renderer->GetMaxLightCount();
	size_t maxShadowCastingCount = drawStencilShadows ? m_Attributes["maxShadowCasters"] : 0;

	size_t count = 0;
	size_t shadowCount = 0;
	for(auto& e : m_LightList) {
		if(e.node->IsTrulyVisible()) {
			illuminating.PushBack(SceneData::LightEntry(e.light, e.node));
			if(e.light->IsShadowCasting() && shadowCount < maxShadowCastingCount) {
				shadowCasting.PushBack(SceneData::LightEntry(e.light, e.node));
				++shadowCount;
			} else {
				nonShadowCasting.PushBack(SceneData::LightEntry(e.light, e.node));
			}
			++count;
			if(count >= maxLightCount)
				break;
		}
	}

	video::PipelineOverwriteToken pot;

	//-------------------------------------------------------------------------
	// Skyboxes
	sceneData.pass = ERenderPass::SkyBox;
	EnableOverwrite(ERenderPass::SkyBox, pot);

	for(auto& e : m_SkyBoxList) {
		if(e.node->IsTrulyVisible())
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
	}

	// Ambient pass
	sceneData.pass = ERenderPass::Solid;
	for(auto& e : m_SolidNodeList) {
		if(e.node->IsTrulyVisible() && !IsCulled(e.node, e.renderable))
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}

	DisableOverwrite(ERenderPass::Solid, pot);
	if(drawStencilShadows)
		m_Renderer->PopPipelineOverwrite(&pot);

	// Shadow pass for each shadow casting light
	if(drawStencilShadows) {
		for(auto shadowLight : shadowCasting) {
			m_Renderer->ClearLights();
			AddDriverLight(shadowLight.node, shadowLight.light);

			m_StencilShadowRenderer.Begin(m_ActiveCameraNode->GetAbsolutePosition(), m_ActiveCameraNode->GetAbsoluteTransform().TransformDir(math::Vector3F::UNIT_Y));
			for(auto& e : m_SolidNodeList) {
				if(e.node->IsTrulyVisible() && e.node->IsShadowCasting()) {
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
				if(e.node->IsTrulyVisible() && !IsCulled(e.node, e.renderable))
					e.renderable->Render(e.node, m_Renderer, sceneData);
			}

			m_Renderer->PopPipelineOverwrite(&pot);
			DisableOverwrite(ERenderPass::Solid, pot);

			m_Renderer->ClearStencil();
		}
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
			if(e.node->IsTrulyVisible() && !IsCulled(e.node, e.renderable))
				e.renderable->Render(e.node, m_Renderer, sceneData);
		}

		m_Renderer->PopPipelineOverwrite(&pot);
		DisableOverwrite(ERenderPass::Solid, pot);
	}

	// Add shadow casting light for remaining render jobs
	for(auto illum : shadowCasting)
		AddDriverLight(illum.node, illum.light);

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
		if(e.node->IsTrulyVisible() && !IsCulled(e.node, e.renderable))
			e.renderable->Render(e.node, m_Renderer, sceneData);
	}

	DisableOverwrite(ERenderPass::Transparent, pot);
	DisableOverwrite(ERenderPass::SolidAndTransparent, pot);
}

void SceneManagerImpl::AddDriverLight(Node* n, Light* l)
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
