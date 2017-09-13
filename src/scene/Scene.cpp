#include "scene/Scene.h"

#include "core/ReferableFactory.h"
#include "core/lxAlgorithm.h"

#include "resources/ResourceSystem.h"

#include "video/mesh/VideoMesh.h"

#include "scene/Node.h"

#include "scene/components/Camera.h"
#include "scene/components/SceneMesh.h"
#include "scene/components/Light.h"
#include "scene/components/Fog.h"
#include "scene/components/SkyBox.h"

#include "scene/components/RotationAnimator.h"
#include "scene/components/LinearMoveAnimator.h"
#include "scene/components/FirstPersonCameraControl.h"

#include "scene/query/LineQuery.h"
#include "scene/query/VolumeQuery.h"

#include "scene/collider/SphereCollider.h"
#include "scene/collider/BoxCollider.h"
#include "scene/collider/MeshCollider.h"

namespace lux
{
namespace scene
{

////////////////////////////////////////////////////////////////////////////////////

Scene::Scene()
{
	m_Root = LUX_NEW(Node)(this, true);
}

Scene::~Scene()
{
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::Clear()
{
	ClearDeletionQueue();

	m_CameraList.Clear();
	m_LightList.Clear();
	m_FogList.Clear();

	m_AnimatedNodes.Clear();

	m_Root->RemoveAllChildren();
}

void Scene::AddToDeletionQueue(Node* node)
{
	if(node)
		m_DeletionQueue.PushBack(node);
}

void Scene::ClearDeletionQueue()
{
	for(auto it = m_DeletionQueue.First(); it != m_DeletionQueue.End(); ++it)
		(*it)->Remove();

	m_DeletionQueue.Clear();
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

StrongRef<Node> Scene::AddMesh(const io::Path& path)
{
	return AddNode(CreateMesh(path));
}

StrongRef<Node> Scene::AddMesh(video::Mesh* mesh)
{
	return AddNode(CreateMesh(mesh));
}

StrongRef<Node> Scene::AddSkyBox(const video::Colorf& color)
{
	return AddNode(CreateSkyBox(color));
}

StrongRef<Node> Scene::AddSkyBox(video::CubeTexture* skyTexture)
{
	return AddNode(CreateSkyBox(skyTexture));
}

StrongRef<Node> Scene::AddLight(video::ELightType lightType, video::Color color)
{
	return AddNode(CreateLight(lightType, color));
}

StrongRef<Node> Scene::AddFog(const video::Colorf& color, float start, float end)
{
	return AddNode(CreateFog(color, start, end));
}

StrongRef<Node> Scene::AddCamera()
{
	return AddNode(CreateCamera());
}

StrongRef<Camera> Scene::CreateCamera() const
{
	return CreateComponent(SceneComponentType::Camera);
}

StrongRef<Mesh> Scene::CreateMesh(const io::Path& path) const
{
	return CreateMesh(core::ResourceSystem::Instance()->GetResource(core::ResourceType::Mesh, path).As<video::Mesh>());
}

StrongRef<Mesh> Scene::CreateMesh(video::Mesh* mesh) const
{
	StrongRef<Mesh> out = CreateComponent(SceneComponentType::Mesh);
	out->SetMesh(mesh);

	return out;
}

StrongRef<SkyBox> Scene::CreateSkyBox(const video::Colorf& color) const
{
	StrongRef<SkyBox> out = CreateComponent(SceneComponentType::SkyBox);
	out->GetMaterial(0)->SetDiffuse(color);
	return out;
}

StrongRef<SkyBox> Scene::CreateSkyBox(video::CubeTexture* skyTexture) const
{
	StrongRef<SkyBox> out = CreateComponent(SceneComponentType::SkyBox);
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<Light> Scene::CreateLight(video::ELightType lightType, video::Color color) const
{
	StrongRef<Light> light = CreateComponent(SceneComponentType::Light);
	light->SetLightType(lightType);
	light->SetColor(color);
	return light;
}

StrongRef<Fog> Scene::CreateFog(const video::Colorf& color, float start, float end) const
{
	StrongRef<Fog> fog = CreateComponent(SceneComponentType::Fog);
	fog->SetFogType(video::EFogType::Linear);
	fog->SetStart(start);
	fog->SetEnd(end);
	fog->SetColor(color);

	return fog;
}

StrongRef<RotationAnimator> Scene::CreateRotator(const math::Vector3F& axis, math::AngleF rotSpeed) const
{
	StrongRef<RotationAnimator> out = CreateComponent(SceneComponentType::Rotation);
	out->SetAxis(axis);
	out->SetRotationSpeed(rotSpeed);

	return out;
}

StrongRef<LinearMoveAnimator> Scene::CreateLinearMover(const math::Line3F& line, float duration) const
{
	StrongRef<LinearMoveAnimator> out = CreateComponent(SceneComponentType::LinearMove);
	out->SetData(line, duration);

	return out;
}

StrongRef<FirstPersonCameraControl> Scene::CreateFirstPersonCameraControl(float moveSpeed, math::AngleF rotSpeed, bool noVerticalMovement) const
{
	StrongRef<FirstPersonCameraControl> out = CreateComponent(SceneComponentType::FirstPersonCameraControl);
	out->SetMoveSpeed(moveSpeed);
	out->SetRotationSpeed(rotSpeed);
	out->AllowVerticalMovement(!noVerticalMovement);

	return out;
}

StrongRef<Component> Scene::CreateComponent(core::Name type) const
{
	return core::ReferableFactory::Instance()->Create(type);
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Collider> Scene::CreateMeshCollider(video::Mesh* mesh) const
{
	return LUX_NEW(MeshCollider)(mesh);
}

StrongRef<Collider> Scene::CreateBoundingBoxCollider() const
{
	return LUX_NEW(BoundingBoxCollider)();
}

StrongRef<Collider> Scene::CreateBoundingSphereCollider() const
{
	return LUX_NEW(BoundingSphereCollider)();
}

StrongRef<Collider> Scene::CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) const
{
	return LUX_NEW(BoxCollider)(halfSize, trans);
}

StrongRef<Collider> Scene::CreateSphereCollider(const math::Vector3F& center, float radius) const
{
	return LUX_NEW(SphereCollider)(center, radius);
}

////////////////////////////////////////////////////////////////////////////////////

Node* Scene::GetRoot() const
{
	return m_Root;
}

bool Scene::IsEmpty() const
{
	return !m_Root->HasChildren();
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterCamera(Camera* camera)
{
	m_CameraList.PushBack(camera);
}

void Scene::UnregisterCamera(Camera* camera)
{
	auto it = core::LinearSearch(camera, m_CameraList);
	if(it != m_CameraList.End())
		m_CameraList.Erase(it);
}

void Scene::RegisterLight(Light* light)
{
	m_LightList.PushBack(light);
}

void Scene::UnregisterLight(Light* light)
{
	auto it = core::LinearSearch(light, m_LightList);
	if(it != m_LightList.End())
		m_LightList.Erase(it);
}

void Scene::RegisterFog(Fog* fog)
{
	m_FogList.PushBack(fog);
}

void Scene::UnregisterFog(Fog* fog)
{
	auto it = core::LinearSearch(fog, m_FogList);
	if(it != m_FogList.End())
		m_FogList.Erase(it);
}

void Scene::RegisterAnimated(Node* node)
{
	m_AnimatedNodes.Insert(node);
}

void Scene::UnregisterAnimated(Node* node)
{
	m_AnimatedNodes.Erase(node);
}

const core::Array<Camera*>& Scene::GetCameraList() const
{
	return m_CameraList;
}

const core::Array<Light*>& Scene::GetLightList() const
{
	return m_LightList;
}

const core::Array<Fog*>& Scene::GetFogList() const
{
	return m_FogList;
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::SetAmbient(const video::Colorf& ambient)
{
	m_AmbientColor = ambient;
}

const video::Colorf& Scene::GetAmbient() const
{
	return m_AmbientColor;
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto node : m_AnimatedNodes)
		node->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

static void VisitRenderablesRec(Node* node, RenderableVisitor* visitor, bool noDebug)
{
	node->VisitRenderables(visitor, noDebug);

	for(auto child : node->Children())
		VisitRenderablesRec(child, visitor, noDebug);
}

void Scene::VisitRenderables(RenderableVisitor* visitor, bool noDebug, Node* root)
{
	if(!root)
		root = m_Root;
	VisitRenderablesRec(root, visitor, noDebug);
}

} // namespace scene
} // namespace lux