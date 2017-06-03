#include "scene/SceneManagerImpl.h"
#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/RenderTarget.h"
#include "video/PipelineSettings.h"

#include "video/MaterialLibrary.h"
#include "video/images/ImageSystem.h"
#include "video/mesh/MeshSystem.h"
#include "video/mesh/VideoMesh.h"
#include "resources/ResourceSystem.h"
#include "io/FileSystem.h"
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

namespace lux
{
namespace scene
{

SceneManagerImpl::SceneManagerImpl(video::VideoDriver* driver,
	video::ImageSystem* imagSys,
	io::FileSystem* fileSystem,
	core::ReferableFactory* refFactory,
	video::MeshSystem* meshCache,
	core::ResourceSystem* resourceSystem,
	video::MaterialLibrary* matLib) :
	m_CollectedRoot(nullptr),
	m_Driver(driver),
	m_Filesystem(fileSystem),
	m_MeshSystem(meshCache),
	m_RefFactory(refFactory),
	m_ImagSys(imagSys),
	m_ResourceSystem(resourceSystem),
	m_MatLib(matLib)
{
	m_RootSceneNode = LUX_NEW(Node)(this, true);
	m_Renderer = m_Driver->GetRenderer();

	m_Overwrites.Resize((size_t)ERenderPass::COUNT);

	m_Fog.isActive = false;
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
	m_EventReceivers.Clear();

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

StrongRef<Camera> SceneManagerImpl::CreateCamera()
{
	return CreateComponent(SceneComponentType::Camera);
}

StrongRef<Mesh> SceneManagerImpl::CreateMesh(const io::path& path)
{
	return CreateMesh(m_ResourceSystem->GetResource(core::ResourceType::Mesh, path).As<video::Mesh>());
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

	// Use a simple solid matrial for the sky box
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<Light> SceneManagerImpl::CreateLight()
{
	return CreateComponent(SceneComponentType::Light);
}

StrongRef<RotationAnimator> SceneManagerImpl::CreateRotator(const math::vector3f& axis, math::anglef rotSpeed)
{
	StrongRef<RotationAnimator> out = CreateComponent(SceneComponentType::Rotation);
	out->SetAxis(axis);
	out->SetRotationSpeed(rotSpeed);

	return out;
}

StrongRef<LinearMoveAnimator> SceneManagerImpl::CreateLinearMover(const math::line3df& line, float duration)
{
	StrongRef<LinearMoveAnimator> out = CreateComponent(SceneComponentType::LinearMove);
	out->SetData(line, duration);

	return out;
}

StrongRef<CameraControl> SceneManagerImpl::CreateCameraControl(float moveSpeed, math::anglef rotSpeed, bool noVerticalMovement)
{
	StrongRef<CameraControl> out = CreateComponent(SceneComponentType::CameraControl);
	out->SetMoveSpeed(moveSpeed);
	out->SetRotationSpeed(rotSpeed);
	out->AllowVerticalMovement(!noVerticalMovement);

	return out;
}

StrongRef<Component> SceneManagerImpl::CreateComponent(core::Name type)
{
	return m_RefFactory->Create(ReferableType::SceneNodeComponent, type);
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

StrongRef<Collider> SceneManagerImpl::CreateBoxCollider(const math::vector3f& halfSize, const math::Transformation& trans)
{
	return LUX_NEW(BoxCollider)(halfSize, trans);
}

StrongRef<Collider> SceneManagerImpl::CreateSphereCollider(const math::vector3f& center, float radius)
{
	return LUX_NEW(SphereCollider)(center, radius);
}

////////////////////////////////////////////////////////////////////////////////////

Node* SceneManagerImpl::GetRoot()
{
	return m_RootSceneNode;
}

StrongRef<Node> SceneManagerImpl::GetActiveCameraNode()
{
	return m_ActiveCameraNode;
}

StrongRef<Camera> SceneManagerImpl::GetActiveCamera()
{
	return m_ActiveCamera;
}

////////////////////////////////////////////////////////////////////////////////////

void SceneManagerImpl::AnimateAll(float secsPassed)
{
	m_RootSceneNode->Animate(secsPassed);
}

bool SceneManagerImpl::DrawAll(bool beginScene, bool endScene)
{
	auto oldActiveCamNode = m_ActiveCameraNode;
	auto oldActiveCam = m_ActiveCamera;

	ClearDeletionQueue();

	// Collect "real" camera nodes
	auto camList = m_CameraList;
	if(camList.Size() == 0)
		return false;

	auto newEnd = core::RemoveIf(camList.First(), camList.End(),
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
			if(!m_SkyBoxList.IsEmpty()) {
				clearZ = true;
				clearColor = true;
			}
			m_Renderer->BeginScene(clearColor, clearZ, video::Color::Black, 1.0f);
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
	auto it = core::LinearSearch(entry, m_CameraList.First(), m_CameraList.End());
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
	auto it = core::LinearSearch(entry, m_LightList.First(), m_LightList.End());
	if(it != m_LightList.End())
		m_LightList.Erase(it);
}

void SceneManagerImpl::RegisterEventReceiver(input::EventReceiver* receiver)
{
	m_EventReceivers.PushBack(receiver);
}

void SceneManagerImpl::UnregisterEventReceiver(input::EventReceiver* receiver)
{
	for(auto it = m_EventReceivers.First(); it != m_EventReceivers.End(); ++it) {
		if(*it == receiver) {
			m_EventReceivers.Erase(it);
			return;
		}
	}
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

void SceneManagerImpl::AddPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over)
{
	size_t id = GetPassId(pass);
	m_Overwrites[id].PushBack(over);
}

void SceneManagerImpl::RemovePipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over)
{
	size_t id = GetPassId(pass);
	for(auto it = m_Overwrites[id].First(); it != m_Overwrites[id].End(); ++it) {
		if(*it == over) {
			m_Overwrites[id].Erase(it);
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////

video::VideoDriver* SceneManagerImpl::GetDriver() const
{
	return m_Driver;
}

video::Renderer* SceneManagerImpl::GetRenderer() const
{
	return m_Renderer;
}

io::FileSystem* SceneManagerImpl::GetFileSystem() const
{
	return m_Filesystem;
}

video::MaterialLibrary* SceneManagerImpl::GetMaterialLibrary() const
{
	return m_MatLib;
}

video::ImageSystem* SceneManagerImpl::GetImageSystem() const
{
	return m_ImagSys;
}

core::ReferableFactory* SceneManagerImpl::GetReferableFactory() const
{
	return m_RefFactory;
}

core::ResourceSystem* SceneManagerImpl::GetResourceSystem() const
{
	return m_ResourceSystem;
}

video::MeshSystem* SceneManagerImpl::GetMeshSystem() const
{
	return m_MeshSystem;
}

////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////

bool SceneManagerImpl::OnEvent(const input::Event& event)
{
	for(auto it = m_EventReceivers.First(); it != m_EventReceivers.End(); ++it) {
		if((*it)->OnEvent(event))
			return true;
	}

	return false;
}

void SceneManagerImpl::EnableOverwrite(ERenderPass pass)
{
	size_t id = GetPassId(pass);
	for(auto it = m_Overwrites[id].First(); it != m_Overwrites[id].End(); ++it)
		m_Renderer->PushPipelineOverwrite(*it);
}

void SceneManagerImpl::DisableOverwrite(ERenderPass pass)
{
	size_t id = GetPassId(pass);
	for(auto it = m_Overwrites[id].First(); it != m_Overwrites[id].End(); ++it)
		m_Renderer->PopPipelineOverwrite();
}

size_t SceneManagerImpl::GetPassId(ERenderPass pass) const
{
	return (size_t)pass;
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

	for(auto it = node->GetChildrenFirst(); it != node->GetChildrenEnd(); ++it)
		CollectRenderablesRec(*it, collector, noDebug);
}

void SceneManagerImpl::AddRenderEntry(Node* n, Renderable* r)
{
	auto pass = r->GetRenderPass();
	if(pass == ERenderPass::SkyBox) {
		m_SkyBoxList.PushBack(RenderEntry(n, r));
	} else if(pass == ERenderPass::Solid) {
		m_SolidNodeList.PushBack(RenderEntry(n, r));
	} else if(pass == ERenderPass::Transparent) {
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r));
	} else if(pass == ERenderPass::SolidAndTransparent) {
		m_SolidNodeList.PushBack(RenderEntry(n, r));
		m_TransparentNodeList.PushBack(DistanceRenderEntry(n, r));
	} else if(pass == ERenderPass::None) {
		(void)0;
	} else {
		throw core::Exception("Unknown render pass used");
	}
}

void SceneManagerImpl::DrawScene()
{
	m_Renderer->SetFog(m_Fog);

	//-------------------------------------------------------------------------
	// The lights
	m_Renderer->GetParam("ambient") = m_AmbientColor;

	m_Renderer->ClearLights();

	size_t maxLightCount = m_Renderer->GetMaxLightCount();
	maxLightCount = math::Min(maxLightCount, m_LightList.Size());

	size_t count = 0;
	for(auto it = m_LightList.First(); it != m_LightList.End(); ++it) {
		if(it->node->IsTrulyVisible()) {
			it->light->Render(m_Renderer, it->node);
			++count;
			if(count == maxLightCount)
				break;
		}
	}

	//-------------------------------------------------------------------------
	// Skyboxes
	EnableOverwrite(ERenderPass::SkyBox);

	for(auto it = m_SkyBoxList.First(); it != m_SkyBoxList.End(); ++it) {
		if(it->node->IsTrulyVisible())
			it->renderable->Render(it->node, m_Renderer, ERenderPass::SkyBox);
	}

	DisableOverwrite(ERenderPass::SkyBox);

	//-------------------------------------------------------------------------
	// Solid objects
	EnableOverwrite(ERenderPass::SolidAndTransparent);
	EnableOverwrite(ERenderPass::Solid);

	for(auto it = m_SolidNodeList.First(); it != m_SolidNodeList.End(); ++it) {
		if(it->node->IsTrulyVisible())
			it->renderable->Render(it->node, m_Renderer, ERenderPass::Solid);
	}

	DisableOverwrite(ERenderPass::Solid);

	//-------------------------------------------------------------------------
	// Transparent objects
	EnableOverwrite(ERenderPass::Transparent);

	// Update the distances in the transparent nodes
	for(auto it = m_TransparentNodeList.First(); it != m_TransparentNodeList.End(); ++it)
		it->UpdateDistance(m_AbsoluteCamPos);

	// Sort by distance
	m_TransparentNodeList.Sort();

	for(auto it = m_TransparentNodeList.First(); it != m_TransparentNodeList.End(); ++it) {
		if(it->node->IsTrulyVisible())
			it->renderable->Render(it->node, m_Renderer, ERenderPass::Transparent);
	}

	DisableOverwrite(ERenderPass::Transparent);
	DisableOverwrite(ERenderPass::SolidAndTransparent);
}

} // namespace scene
} // namespace lux
