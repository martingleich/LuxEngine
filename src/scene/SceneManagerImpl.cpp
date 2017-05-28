#include "SceneManagerImpl.h"
#include "video/VideoDriver.h"
#include "video/Renderer.h"
#include "video/RenderTarget.h"
#include "video/MaterialLibrary.h"
#include "video/MaterialRenderer.h"
#include "video/images/ImageSystem.h"
#include "video/PipelineSettings.h"

#include "scene/mesh/MeshSystem.h"
#include "io/FileSystem.h"

#include "resources/ResourceSystem.h"

#include "core/ReferableFactory.h"

#include "scene/nodes/CameraSceneNode.h"
#include "scene/nodes/MeshSceneNode.h"
#include "scene/nodes/LightSceneNode.h"
#include "scene/nodes/SkyBoxSceneNodeImpl.h"

#include "scene/components/RotationAnimatorImpl.h"
#include "scene/components/LinearMoveAnimator.h"
#include "scene/components/CameraFPSAnimatorImpl.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"

#include "scene/collider/SphereCollider.h"
#include "scene/collider/BoxCollider.h"
#include "scene/collider/MeshCollider.h"

#include "core/ReferableRegister.h"

#include "core/Logger.h"

namespace lux
{
namespace scene
{

class RootSceneNode : public SceneNode
{
public:
	RootSceneNode() {}
	RootSceneNode(SceneManager* mngr)
	{
		SetSceneManager(mngr);
	}

	void Render()
	{
	}
	core::Name GetReferableSubType() const
	{
		return SceneNodeType::Root;
	}

	StrongRef<Referable> Clone() const
	{
		return new RootSceneNode(nullptr);
	}
};

LUX_REGISTER_REFERABLE_CLASS(RootSceneNode)

SceneManagerImpl::SSolidNodeEntry::SSolidNodeEntry() : texture(nullptr), node(nullptr)
{
}
SceneManagerImpl::SSolidNodeEntry::SSolidNodeEntry(SceneNode* node) : texture(nullptr), node(node)
{
	if(node->GetMaterialCount()) {
		if(node->GetMaterial(0)->GetTextureCount())
			texture = ((video::TextureLayer)node->GetMaterial(0)->Layer(0)).texture;
	}
}

bool SceneManagerImpl::SSolidNodeEntry::operator<(const SSolidNodeEntry& other) const
{
	return (texture < other.texture);
}
bool SceneManagerImpl::SSolidNodeEntry::operator==(const SSolidNodeEntry& other) const
{
	return (texture == other.texture);
}

SceneManagerImpl::STransparentNodeEntry::STransparentNodeEntry() : distance(-1.0f), node(nullptr)
{
}
SceneManagerImpl::STransparentNodeEntry::STransparentNodeEntry(SceneNode* node, math::vector3f vCameraPos) : node(node)
{
	distance = node->GetAbsolutePosition().GetDistanceToSq(vCameraPos);
}

bool SceneManagerImpl::STransparentNodeEntry::operator<(const STransparentNodeEntry& other) const
{
	return (distance < other.distance);
}
bool SceneManagerImpl::STransparentNodeEntry::operator==(const STransparentNodeEntry& other) const
{
	return (distance == other.distance);
}

bool SceneManagerImpl::CameraEntry::operator<(const CameraEntry& other) const
{
	return camera->GetRenderPriority() > other.camera->GetRenderPriority();
}

bool SceneManagerImpl::CameraEntry::operator==(const CameraEntry& other) const
{
	return camera->GetRenderPriority() == other.camera->GetRenderPriority();
}

SceneManagerImpl::SceneManagerImpl(video::VideoDriver* driver,
	video::ImageSystem* imagSys,
	io::FileSystem* fileSystem,
	core::ReferableFactory* refFactory,
	MeshSystem* meshCache,
	core::ResourceSystem* resourceSystem,
	video::MaterialLibrary* matLib)
	: m_Driver(driver),
	m_Filesystem(fileSystem),
	m_MeshSystem(meshCache),
	m_RefFactory(refFactory),
	m_ImagSys(imagSys),
	m_ResourceSystem(resourceSystem),
	m_MatLib(matLib),
	m_CurrentRenderPass(ESNRP_NONE),
	m_AmbientColor(0)
{
	m_RootSceneNode = LUX_NEW(RootSceneNode)(this);
	m_Renderer = m_Driver->GetRenderer();

	m_Overwrites.Resize(ESNRP_COUNT);

	m_Fog.isActive = false;
}

SceneManagerImpl::~SceneManagerImpl()
{
	ClearDeletionQueue();

	m_RootSceneNode->RemoveAllChildren();
	m_RootSceneNode = nullptr;
}

StrongRef<SceneNode> SceneManagerImpl::AddSceneNode(core::Name type, SceneNode* parent)
{
	if(!parent)
		parent = GetRootSceneNode();

	StrongRef<SceneNode> out = m_RefFactory->Create(ReferableType::SceneNode, type);
	if(out) {
		if(!out->SetParent(parent))
			return nullptr;
	} else {
		log::Error("Can't created scene node type ~s.", type);
	}

	return out;
}

StrongRef<SceneNodeComponent> SceneManagerImpl::AddComponent(core::Name type, SceneNode* node)
{
	StrongRef<SceneNodeComponent> out = m_RefFactory->Create(ReferableType::SceneNodeComponent, type);
	if(out) {
		node->AddComponent(out);
	} else {
		log::Error("Can't created scene node component type ~s.", type);
	}

	return out;
}

StrongRef<CameraSceneNode> SceneManagerImpl::AddCameraSceneNode(const math::vector3f& position,
	const math::vector3f& direction,
	bool makeActive)
{
	StrongRef<CameraSceneNode> out = AddSceneNode(SceneNodeType::Camera);
	if(!out)
		return nullptr;

	out->SetPosition(position);
	out->SetDirection(direction);

	if(makeActive)
		SetActiveCamera(out);

	return out;
}

StrongRef<MeshSceneNode> SceneManagerImpl::AddMeshSceneNode(Mesh* mesh,
	bool addIfMeshIsEmpty,
	const math::vector3f& position,
	const math::quaternionf& orientation,
	float scale)
{
	if(!addIfMeshIsEmpty && !mesh)
		return nullptr;

	StrongRef<MeshSceneNode> out = AddSceneNode(SceneNodeType::Mesh);
	if(!out)
		return nullptr;
	out->SetMesh(mesh);
	out->SetPosition(position);
	out->SetOrientation(orientation);
	out->SetScale(scale);

	return out;
}

StrongRef<SceneNode> SceneManagerImpl::AddSkyBoxSceneNode(video::CubeTexture* skyTexture)
{
	StrongRef<SkyBoxSceneNodeImpl> out = AddSceneNode(SceneNodeType::SkyBox);
	if(!out)
		return nullptr;
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<LightSceneNode> SceneManagerImpl::AddLightSceneNode(const math::vector3f position,
	const video::Colorf color, float range)
{
	StrongRef<LightSceneNode> out = AddSceneNode(SceneNodeType::Light);
	if(!out)
		return nullptr;
	out->SetPosition(position);
	out->SetColor(color);
	out->SetRange(range);

	return out;
}

StrongRef<SceneNode> SceneManagerImpl::AddEmptySceneNode(SceneNode* parent)
{
	return AddSceneNode(SceneNodeType::Empty, parent);
}

StrongRef<SceneNodeComponent> SceneManagerImpl::AddRotationAnimator(SceneNode* addTo, const math::vector3f& axis, math::anglef rotSpeed)
{
	if(!addTo)
		return nullptr;

	StrongRef<scene::SceneNodeAnimatorRotationImpl> animator = AddComponent(scene::SceneNodeComponentType::Rotation, addTo);
	animator->SetAxis(axis);
	animator->SetRotationSpeed(rotSpeed);

	return animator;
}

StrongRef<CameraFPSAnimator> SceneManagerImpl::AddCameraFPSAnimator(CameraSceneNode* addTo, float moveSpeed, math::anglef rotSpeed,
	bool noVerticalMovement,
	math::anglef maxVerticalAngle)
{
	if(!addTo)
		return nullptr;

	StrongRef<scene::CameraFPSAnimator> animator = AddComponent(scene::SceneNodeComponentType::CameraFPS, addTo);
	animator->SetMoveSpeed(moveSpeed);
	animator->SetRotationSpeed(rotSpeed);
	animator->AllowVerticalMovement(!noVerticalMovement);
	animator->SetMaxVerticalAngle(maxVerticalAngle);

	return animator;
}

StrongRef<SceneNodeComponent> SceneManagerImpl::AddLinearMoveAnimator(SceneNode* addTo,
	const math::line3df& line,
	float duration,
	bool jumpBack,
	u32 count)
{
	if(!addTo)
		return nullptr;

	StrongRef<LinearMoveComponent> animator = AddComponent(scene::SceneNodeComponentType::LinearMove, addTo);
	animator->Init(line, duration, jumpBack, count);

	return animator;
}

StrongRef<Collider> SceneManagerImpl::CreateMeshCollider(Mesh* mesh)
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

StrongRef<Collider> SceneManagerImpl::CreateSphereCollider(const math::vector3f& center, float radius)
{
	return LUX_NEW(SphereCollider)(center, radius);
}

StrongRef<Collider> SceneManagerImpl::CreateBoxCollider(const math::vector3f& halfSize, const math::Transformation& trans)
{
	return LUX_NEW(BoxCollider)(halfSize, trans);
}

video::VideoDriver* SceneManagerImpl::GetDriver()
{
	return m_Driver;
}

video::Renderer* SceneManagerImpl::GetRenderer()
{
	return m_Renderer;
}

video::MaterialLibrary* SceneManagerImpl::GetMaterialLibrary()
{
	return m_MatLib;
}

io::FileSystem* SceneManagerImpl::GetFileSystem()
{
	return m_Filesystem;
}

MeshSystem* SceneManagerImpl::GetMeshSystem()
{
	return m_MeshSystem;
}

video::ImageSystem* SceneManagerImpl::GetImageSystem() const
{
	return m_ImagSys;
}

SceneNode* SceneManagerImpl::GetRootSceneNode()
{
	return m_RootSceneNode;
}

StrongRef<CameraSceneNode> SceneManagerImpl::GetActiveCamera()
{
	return m_ActiveCamera;
}

void SceneManagerImpl::SetActiveCamera(CameraSceneNode* camera)
{
	m_ActiveCamera = camera;
}

void SceneManagerImpl::Render()
{
}

void SceneManagerImpl::RegisterCamera(CameraSceneNode* camera)
{
	m_CameraList.PushBack(camera);
}

void SceneManagerImpl::UnRegisterCamera(CameraSceneNode* camera)
{
	auto it = core::LinearSearch(camera, m_CameraList.First(), m_CameraList.End());
	if(it != m_CameraList.End())
		m_CameraList.Erase(it);
}

bool SceneManagerImpl::RegisterNodeForRendering(SceneNode* node, ESceneNodeRenderPass renderPass)
{
	bool wasTaken = false;

	// Wann soll der Scene-node gezeichnet werden
	switch(renderPass) {
	case ESNRP_SKY_BOX:
		m_SkyBoxList.PushBack(node);
		wasTaken = true;
		break;
	case ESNRP_SOLID:
		m_SolidNodeList.PushBack(SSolidNodeEntry(node));
		wasTaken = true;
		break;

	case ESNRP_TRANSPARENT:
		m_TransparentNodeList.PushBack(STransparentNodeEntry(node, m_AbsoluteCamPos));
		wasTaken = true;
		break;

	case ESNRP_AUTOMATIC:
	{
		const size_t count = node->GetMaterialCount();
		for(size_t i = 0; i < count; ++i) {
			// Ist ein material transparent, dann gilt der ganze Knoten als transparent
			video::MaterialRenderer* renderer = node->GetMaterial(i)->GetRenderer();
			if(renderer && renderer->GetRequirements() == video::MaterialRenderer::ERequirement::Transparent) {
				m_TransparentNodeList.PushBack(STransparentNodeEntry(node, m_AbsoluteCamPos));
				wasTaken = true;
				break;
			}
		}

		// Kein transparentes material -> Knoten solid
		if(!wasTaken) {
			m_SolidNodeList.PushBack(SSolidNodeEntry(node));
			wasTaken = true;
		}
	}
	break;

	case ESNRP_LIGHT:
		m_LightList.PushBack((LightSceneNode*)node);
		wasTaken = true;
		break;
	default:
		wasTaken = false;
		break;
	}

	return wasTaken;
}


// Animiert alles
void SceneManagerImpl::AnimateAll(float secsPassed)
{
	// Es reicht die Animationsfunktion des Wurzelknotens aufzurufen
	GetRootSceneNode()->Animate(secsPassed);
}


// Zeichnet alles
bool SceneManagerImpl::DrawAll(bool beginScene, bool endScene)
{
	ClearDeletionQueue();

	m_RenderRoot = nullptr;

	auto camList = m_CameraList;
	if(camList.Size() == 0)
		return false;

	auto newEnd = core::RemoveIf(camList.First(), camList.End(),
		[](const CameraEntry& e) -> bool {
		return !e.camera->IsTrulyVisible();
	});
	camList.Resize(core::IteratorDistance(camList.First(), newEnd));

	camList.Sort();

	if(!beginScene) {
		if(m_Renderer->GetRenderTarget() != camList[0].camera->GetRenderTarget()) {
			log::Error("Scenemanager.DrawAll: Already started scene uses diffrent rendertarget than first camera.");
			return false;
		}
	}

	for(auto it = camList.First(); it != camList.End(); ++it) {
		m_ActiveCamera = it->camera;
		m_AbsoluteCamPos = m_ActiveCamera->GetAbsoluteTransform().translation;

		m_CurrentRenderPass = ESNRP_CAMERA;

		if(it != camList.First() || beginScene) {
			m_Renderer->SetRenderTarget(m_ActiveCamera->GetRenderTarget());
			auto backgroundColor = m_ActiveCamera->GetBackgroundColor();
			bool clearColor = true;
			if(backgroundColor.GetAlpha() == 0)
				clearColor = false;
			m_Renderer->BeginScene(clearColor, true, backgroundColor, 1.0f);
		}

		m_ActiveCamera->Render();

		DrawScene(m_ActiveCamera->GetVirtualRoot());

		if(it != camList.Last() || endScene)
			m_Renderer->EndScene();
	}

	m_SkyBoxList.Clear();
	m_SolidNodeList.Clear();
	m_TransparentNodeList.Clear();
	m_LightList.Clear();

	return true;
}

void SceneManagerImpl::DrawScene(SceneNode* root)
{
	if(!root)
		root = m_RootSceneNode;

	if(root != m_RenderRoot) {
		m_SkyBoxList.Clear();
		m_SolidNodeList.Clear();
		m_TransparentNodeList.Clear();
		m_LightList.Clear();

		root->OnRegisterSceneNode();
		m_RenderRoot = root;
	}

	m_Renderer->SetFog(m_Fog);

	//-------------------------------------------------------------------------
	// Die Lichter aktivieren
	m_CurrentRenderPass = ESNRP_LIGHT;

	EnableOverwrite();

	m_Renderer->GetParam("ambient") = m_AmbientColor;

	m_Renderer->ClearLights();

	size_t maxLightCount = m_Renderer->GetMaxLightCount();
	maxLightCount = math::Min(maxLightCount, m_LightList.Size());

	for(size_t i = 0; i < maxLightCount; ++i)
		m_LightList[i]->Render();

	DisableOverwrite();

	//-------------------------------------------------------------------------
	// Die Skyboxes zeichnen
	m_CurrentRenderPass = ESNRP_SKY_BOX;

	EnableOverwrite();

	for(auto it = m_SkyBoxList.First(); it != m_SkyBoxList.End(); ++it)
		(*it)->Render();

	DisableOverwrite();

	//-------------------------------------------------------------------------
	// Die normalen Objekte zeichnen
	m_CurrentRenderPass = ESNRP_SOLID;

	EnableOverwrite();

	// Die Knoten anhand ihrer Haupttextur sortieren
	m_SolidNodeList.Sort();

	for(auto it = m_SolidNodeList.First(); it != m_SolidNodeList.End(); ++it)
		it->node->Render();

	DisableOverwrite();

	//-------------------------------------------------------------------------

	// Die transparenten Objekte zeichnen
	m_CurrentRenderPass = ESNRP_TRANSPARENT;

	EnableOverwrite();

	// Die Knoten anhand ihrer Entfernung zur Kamera sortieren
	m_TransparentNodeList.Sort();

	for(auto it = m_TransparentNodeList.First(); it != m_TransparentNodeList.End(); ++it)
		it->node->Render();

	DisableOverwrite();

	//-------------------------------------------------------------------------

	m_CurrentRenderPass = ESNRP_NONE;
}

void SceneManagerImpl::AddToDeletionQueue(SceneNode* node)
{
	if(node)
		m_DeletionList.PushBack(node);
}

void SceneManagerImpl::ClearDeletionQueue()
{
	for(u32 i = 0; i < m_DeletionList.Size(); ++i)
		m_DeletionList[i]->Remove();

	m_DeletionList.Clear();
}


void SceneManagerImpl::Clear()
{
	GetRootSceneNode()->RemoveAllChildren();
}

ESceneNodeRenderPass SceneManagerImpl::GetActRenderPass() const
{
	return m_CurrentRenderPass;
}

// Fragt einen Scene-node anhand seines Typs ab
SceneNode* SceneManagerImpl::GetSceneNodeByType(core::Name type, SceneNode* start, u32 tags)
{
	if(!start)
		start = GetRootSceneNode();

	if(type == start->GetReferableSubType() && start->HasTag(tags))
		return start;

	for(auto it = start->GetChildrenFirst(); it != start->GetChildrenEnd(); ++it) {
		SceneNode* node = GetSceneNodeByType(type, it.Pointer());
		if(node)
			return node;
	}

	return nullptr;
}

bool SceneManagerImpl::GetSceneNodeArrayByType(core::Name type, core::array<SceneNode*>& array, SceneNode* start, u32 tags)
{
	if(!start)
		start = GetRootSceneNode();

	if(start->GetReferableSubType() == type && start->HasTag(tags))
		array.PushBack(start);

	for(auto it = start->GetChildrenFirst(); it != start->GetChildrenEnd(); ++it)
		GetSceneNodeArrayByType(type, array, it.Pointer());

	return !array.IsEmpty();
}

void SceneManagerImpl::SetAmbient(video::Colorf ambient)
{
	m_AmbientColor = ambient;
}

video::Colorf SceneManagerImpl::GetAmbient()
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

bool SceneManagerImpl::OnEvent(const input::Event& event)
{
	for(auto it = m_EventReceivers.First(); it != m_EventReceivers.End();) {
		SceneNode* node = *it;
		if(node) {
			if(node->OnEvent(event))
				return true;
			++it;
		} else {
			auto next = it;
			++next;
			m_EventReceivers.Erase(it);
			it = next;
		}
	}

	return false;
}


void SceneManagerImpl::RegisterEventReceiver(SceneNode* receiver)
{
	m_EventReceivers.PushBack(receiver);
}

void SceneManagerImpl::UnregisterEventReceiver(SceneNode* receiver)
{
	for(auto it = m_EventReceivers.First(); it != m_EventReceivers.End(); ++it)
		if(*it == receiver) {
			m_EventReceivers.Erase(it);
			return;
		}
}

void SceneManagerImpl::AddPipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over)
{
	m_Overwrites[pass].PushBack(over);
}

void SceneManagerImpl::RemovePipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over)
{
	for(auto it = m_Overwrites[pass].First(); it != m_Overwrites[pass].End(); ++it) {
		if(*it == over) {
			m_Overwrites[pass].Erase(it);
			return;
		}
	}
}

void SceneManagerImpl::EnableOverwrite()
{
	for(auto it = m_Overwrites[m_CurrentRenderPass].First(); it != m_Overwrites[m_CurrentRenderPass].End(); ++it)
		m_Renderer->PushPipelineOverwrite(*it);
}

void SceneManagerImpl::DisableOverwrite()
{
	for(auto it = m_Overwrites[m_CurrentRenderPass].First(); it != m_Overwrites[m_CurrentRenderPass].End(); ++it)
		m_Renderer->PopPipelineOverwrite();
}

core::ReferableFactory* SceneManagerImpl::GetReferableFactory() const
{
	return m_RefFactory;
}

core::ResourceSystem* SceneManagerImpl::GetResourceSystem() const
{
	return m_ResourceSystem;
}


}

}

