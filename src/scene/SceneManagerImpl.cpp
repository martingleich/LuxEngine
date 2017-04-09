#include "SceneManagerImpl.h"

#include "io/FileSystem.h"

#include "video/VideoDriver.h"
#include "video/images/ImageSystem.h"
#include "video/MaterialLibrary.h"

#include "resources/ReferableFactory.h"

#include "scene/nodes/CameraSceneNodeImpl.h"
#include "scene/nodes/MeshSceneNodeImpl.h"
#include "scene/nodes/SkyBoxSceneNodeImpl.h"
#include "scene/nodes/LightSceneNodeImpl.h"
#include "scene/nodes/EmptySceneNodeImpl.h"

#include "scene/components/RotationAnimatorImpl.h"
#include "scene/components/LinearMoveAnimator.h"
#include "scene/components/CameraFPSAnimatorImpl.h"

#include "scene/mesh/MeshCacheImpl.h"

//#include "scene/mesh/MeshLoader3DS.h"
#include "scene/mesh/MeshLoaderOBJ.h"

#include "core/Logger.h"

namespace lux
{
namespace scene
{

class RootSceneNode : public SceneNode
{
public:
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

SceneManagerImpl::SSolidNodeEntry::SSolidNodeEntry() : node(nullptr), texture(nullptr)
{
}
SceneManagerImpl::SSolidNodeEntry::SSolidNodeEntry(SceneNode* node) : node(node), texture(nullptr)
{
	if(node->GetMaterialCount()) {
		if(node->GetMaterial(0).GetTextureCount())
			texture = ((video::MaterialLayer)node->GetMaterial(0).Layer(0)).texture;
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

SceneManagerImpl::STransparentNodeEntry::STransparentNodeEntry() : node(nullptr), distance(-1.0f)
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

SceneManagerImpl::SceneManagerImpl(video::VideoDriver* driver,
	video::ImageSystem* imagSys,
	io::FileSystem* fileSystem,
	core::ReferableFactory* refFactory,
	MeshCache* meshCache,
	core::ResourceSystem* resourceSystem,
	video::MaterialLibrary* matLib)
	: m_Driver(driver),
	m_Filesystem(fileSystem),
	m_MeshCache(meshCache),
	m_ImagSys(imagSys),
	m_RefFactory(refFactory),
	m_MatLib(matLib),
	m_ResourceSystem(resourceSystem),
	m_CurrentRenderPass(ESNRP_NONE),
	m_AmbientColor(0)
{
	if(m_MeshCache == nullptr) {
		m_MeshCache = LUX_NEW(MeshCacheImpl)(resourceSystem, driver);
		//m_ResourceSystem->AddResourceLoader(LUX_NEW(MeshLoader3DS)(this));
		m_ResourceSystem->AddResourceLoader(LUX_NEW(MeshLoaderOBJ)(this));
	}

	m_RefFactory->RegisterType(LUX_NEW(MeshSceneNodeImpl));
	m_RefFactory->RegisterType(LUX_NEW(LightSceneNodeImpl));
	m_RefFactory->RegisterType(LUX_NEW(SkyBoxSceneNodeImpl));
	m_RefFactory->RegisterType(LUX_NEW(CameraSceneNodeImpl));
	m_RefFactory->RegisterType(LUX_NEW(EmptySceneNode));

	m_RefFactory->RegisterType(LUX_NEW(LinearMoveComponent));
	m_RefFactory->RegisterType(LUX_NEW(SceneNodeAnimatorRotationImpl));
	m_RefFactory->RegisterType(LUX_NEW(CameraFPSAnimatorImpl));

	m_RootSceneNode = LUX_NEW(RootSceneNode)(this);

	m_Overwrites.Resize(ESNRP_COUNT);
}

SceneManagerImpl::~SceneManagerImpl()
{
	ClearDeletionQueue();

	m_RootSceneNode->RemoveAllChildren();
	m_RootSceneNode = nullptr;

	m_MeshCache = nullptr;
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

	StrongRef<SceneNodeComponent> animator = LUX_NEW(scene::SceneNodeAnimatorRotationImpl)(axis, rotSpeed);
	addTo->AddComponent(animator);

	return animator;
}

StrongRef<CameraFPSAnimator> SceneManagerImpl::AddCameraFPSAnimator(CameraSceneNode* addTo, float moveSpeed, math::anglef rotSpeed,
	bool noVerticalMovement,
	math::anglef maxVerticalAngle)
{
	if(!addTo)
		return nullptr;

	StrongRef<scene::CameraFPSAnimator> animator = LUX_NEW(scene::CameraFPSAnimatorImpl)(moveSpeed, rotSpeed,
		maxVerticalAngle, noVerticalMovement);
	addTo->AddComponent(animator);

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

	auto animator = LUX_NEW(scene::LinearMoveComponent)(line, duration, jumpBack, count);
	addTo->AddComponent(animator);

	return animator;
}

video::VideoDriver* SceneManagerImpl::GetDriver()
{
	return m_Driver;
}

video::MaterialLibrary* SceneManagerImpl::GetMaterialLibrary()
{
	return m_MatLib;
}

io::FileSystem* SceneManagerImpl::GetFileSystem()
{
	return m_Filesystem;
}

MeshCache* SceneManagerImpl::GetMeshCache()
{
	return m_MeshCache;
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

bool SceneManagerImpl::RegisterNodeForRendering(SceneNode* node, ESceneNodeRenderPass renderPass)
{
	bool wasTaken = false;

	// Wann soll der Scene-node gezeichnet werden
	switch(renderPass) {
	case ESNRP_CAMERA:

		wasTaken = true;

		// Kameras dürfen nur einmal angenommen werden
		for(u32 dwNode = 0; dwNode < m_CameraList.Size(); ++dwNode) {
			if(m_CameraList[dwNode] == node) {
				wasTaken = false;
				break;
			}
		}

		// Knoten hinzufügen
		if(wasTaken)    m_CameraList.Push_Back((CameraSceneNode*&)(node));
		break;

	case ESNRP_SKY_BOX:
		m_SkyBoxList.Push_Back(node);
		wasTaken = true;
		break;
	case ESNRP_SOLID:
		// TODO Culling
		m_SolidNodeList.Push_Back(SSolidNodeEntry(node));
		wasTaken = true;
		break;

	case ESNRP_TRANSPARENT:
		// TODO Culling
		m_TransparentNodeList.Push_Back(STransparentNodeEntry(node, m_AbsoluteCamPos));
		wasTaken = true;
		break;

	case ESNRP_AUTOMATIC:
	{
		// TODO Culling

		const size_t count = node->GetMaterialCount();
		for(size_t i = 0; i < count; ++i) {
			// Ist ein material transparent, dann gilt der ganze Knoten als transparent
			video::MaterialRenderer* renderer = node->GetMaterial(i).GetRenderer();
			if(renderer && renderer->IsTransparent()) {
				m_TransparentNodeList.Push_Back(STransparentNodeEntry(node, m_AbsoluteCamPos));
				wasTaken = true;
				break;
			}
		}

		// Kein transparentes material -> Knoten solid
		if(!wasTaken) {
			m_SolidNodeList.Push_Back(SSolidNodeEntry(node));
			wasTaken = true;
		}
	}
	break;

	case ESNRP_LIGHT:
		m_LightList.Push_Back((LightSceneNode*)node);
		wasTaken = true;
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
void SceneManagerImpl::DrawAll()
{
	if(!m_Driver) return;

	// Alle Knoten registrieren
	GetRootSceneNode()->OnRegisterSceneNode();

	if(m_CameraList.Size() == 0)
		return;

	if(!m_ActiveCamera)
		m_ActiveCamera = m_CameraList[0];

	m_AbsoluteCamPos = m_ActiveCamera->GetAbsoluteTransform().translation;

	//-------------------------------------------------------------------------
	// Die Kameras zeichnen
	m_CurrentRenderPass = ESNRP_CAMERA;

	EnableOverwrite();

	for(auto it = m_CameraList.First(); it != m_CameraList.End(); ++it)
		(*it)->Render();

	// Kameraliste zurücksetzten
	m_CameraList.Resize(0);

	DisableOverwrite();

	//-------------------------------------------------------------------------
	// Die Lichter aktivieren
	m_CurrentRenderPass = ESNRP_LIGHT;

	EnableOverwrite();

	// Das Hintergrundlicht neu setzten
	m_Driver->SetAmbient(m_AmbientColor);

	m_Driver->DeleteAllLights();

	size_t maxLightCount = m_Driver->GetMaximalLightCount();
	maxLightCount = math::Min(maxLightCount, m_LightList.Size());

	// TODO: Lichter nach Kameradistanz sortieren, und am besten passedne Lichter auswählen
	for(size_t i = 0; i < maxLightCount; ++i) {
		m_LightList[i]->Render();
	}

	m_LightList.Resize(0);

	DisableOverwrite();

	//-------------------------------------------------------------------------
	// Die Skyboxes zeichnen
	m_CurrentRenderPass = ESNRP_SKY_BOX;

	EnableOverwrite();

	for(auto it = m_SkyBoxList.First(); it != m_SkyBoxList.End(); ++it)
		(*it)->Render();

	m_SkyBoxList.Resize(0);

	DisableOverwrite();

	//-------------------------------------------------------------------------
	// Die normalen Objekte zeichnen
	m_CurrentRenderPass = ESNRP_SOLID;

	EnableOverwrite();

	// Die Knoten anhand ihrer Haupttextur sortieren
	m_SolidNodeList.Sort();

	for(auto it = m_SolidNodeList.First(); it != m_SolidNodeList.End(); ++it)
		it->node->Render();

	m_SolidNodeList.Resize(0);

	DisableOverwrite();

	//-------------------------------------------------------------------------

	// Die transparenten Objekte zeichnen
	m_CurrentRenderPass = ESNRP_TRANSPARENT;

	EnableOverwrite();

	// Die Knoten anhand ihrer Entfernung zur Kamera sortieren
	m_TransparentNodeList.Sort();

	for(auto it = m_TransparentNodeList.First(); it != m_TransparentNodeList.End(); ++it)
		it->node->Render();

	m_TransparentNodeList.Resize(0);

	DisableOverwrite();

	//-------------------------------------------------------------------------

	m_CurrentRenderPass = ESNRP_NONE;

	ClearDeletionQueue();
}

void SceneManagerImpl::AddToDeletionQueue(SceneNode* node)
{
	if(node)
		m_DeletionList.Push_Back(node);
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

	if(m_Driver)
		m_Driver->Set3DMaterial(video::IdentityMaterial);
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
		array.Push_Back(start);

	for(auto it = start->GetChildrenFirst(); it != start->GetChildrenEnd(); ++it)
		GetSceneNodeArrayByType(type, array, it.Pointer());

	return !array.IsEmpty();
}

void SceneManagerImpl::SetAmbient(video::Color ambient)
{
	m_AmbientColor = ambient;
}

video::Color SceneManagerImpl::GetAmbient()
{
	return m_AmbientColor;
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
	m_EventReceivers.Push_Back(receiver);
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
	m_Overwrites[pass].Push_Back(over);
}

void SceneManagerImpl::RemovePipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over)
{
	for(auto it = m_Overwrites[pass].First(); it != m_Overwrites[pass].End(); ++it) {
		if(it->Flags == over.Flags) {
			m_Overwrites[pass].Erase(it);
			return;
		}
	}
}

void SceneManagerImpl::EnableOverwrite()
{
	for(auto it = m_Overwrites[m_CurrentRenderPass].First(); it != m_Overwrites[m_CurrentRenderPass].End(); ++it) {
		m_Driver->PushPipelineOverwrite(*it);
	}
}

void SceneManagerImpl::DisableOverwrite()
{
	for(auto it = m_Overwrites[m_CurrentRenderPass].First(); it != m_Overwrites[m_CurrentRenderPass].End(); ++it) {
		m_Driver->PopPipelineOverwrite();
	}
}

core::ReferableFactory* SceneManagerImpl::GetReferableFactory() const
{
	return m_RefFactory;
}

core::ResourceSystem* SceneManagerImpl::GetResourceSystem() const
{
	return m_ResourceSystem;
}


}    // namespace scene
}    // namespace lux