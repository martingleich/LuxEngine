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
#include "scene/components/TurntableCameraControl.h"

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

void Scene::RegisterObserver(SceneObserver* observer)
{
	m_Observers.PushBack(observer);
}

void Scene::UnregisterObserver(SceneObserver* observer)
{
	for(auto it = m_Observers.First(); it != m_Observers.End(); ++it) {
		if(*it == observer) {
			m_Observers.Erase(it);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::Clear()
{
	ClearDeletionQueue();

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

StrongRef<Node> Scene::AddSkyBox(const video::ColorF& color)
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

StrongRef<Node> Scene::AddFog(const video::ColorF& color, float start, float end)
{
	return AddNode(CreateFog(color, start, end));
}

StrongRef<Node> Scene::AddCamera()
{
	return AddNode(CreateCamera());
}

StrongRef<Camera> Scene::CreateCamera() const
{
	return CreateComponent(SceneComponentType::Camera).As<Camera>();
}

StrongRef<Mesh> Scene::CreateMesh(const io::Path& path) const
{
	return CreateMesh(core::ResourceSystem::Instance()->GetResource(core::ResourceType::Mesh, path).As<video::Mesh>());
}

StrongRef<Mesh> Scene::CreateMesh(video::Mesh* mesh) const
{
	auto out = CreateComponent(SceneComponentType::Mesh).AsStrong<Mesh>();
	out->SetMesh(mesh);

	return out;
}

StrongRef<SkyBox> Scene::CreateSkyBox(const video::ColorF& color) const
{
	auto out = CreateComponent(SceneComponentType::SkyBox).AsStrong<SkyBox>();
	out->GetMaterial()->SetDiffuse(color);
	return out;
}

StrongRef<SkyBox> Scene::CreateSkyBox(video::CubeTexture* skyTexture) const
{
	auto out = CreateComponent(SceneComponentType::SkyBox).AsStrong<SkyBox>();
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<Light> Scene::CreateLight(video::ELightType lightType, video::Color color) const
{
	auto light = CreateComponent(SceneComponentType::Light).AsStrong<Light>();
	light->SetLightType(lightType);
	light->SetColor(color);
	return light;
}

StrongRef<GlobalFog> Scene::CreateFog(const video::ColorF& color, float start, float end) const
{
	auto fog = CreateComponent(SceneComponentType::GlobalFog).AsStrong<GlobalFog>();
	fog->SetFogType(video::EFogType::Linear);
	fog->SetStart(start);
	fog->SetEnd(end);
	fog->SetColor(color);

	return fog;
}

StrongRef<RotationAnimator> Scene::CreateRotator(const math::Vector3F& axis, math::AngleF rotSpeed) const
{
	auto out = CreateComponent(SceneComponentType::Rotation).AsStrong<RotationAnimator>();
	out->SetAxis(axis);
	out->SetRotationSpeed(rotSpeed);

	return out;
}

StrongRef<LinearMoveAnimator> Scene::CreateLinearMover(const math::Line3F& line, float duration) const
{
	auto out = CreateComponent(SceneComponentType::LinearMove).AsStrong<LinearMoveAnimator>();
	out->SetData(line, duration);

	return out;
}

StrongRef<FirstPersonCameraControl> Scene::CreateFirstPersonCameraControl(float moveSpeed, math::AngleF rotSpeed, bool noVerticalMovement) const
{
	auto out = CreateComponent(SceneComponentType::FirstPersonCameraControl).AsStrong<FirstPersonCameraControl>();
	out->SetMoveSpeed(moveSpeed);
	out->SetRotationSpeed(rotSpeed);
	out->AllowVerticalMovement(!noVerticalMovement);

	return out;
}

StrongRef<TurntableCameraControl> Scene::CreateTurntableCameraControl(const math::Vector3F& orbit, const math::Vector3F& normal) const
{
	auto out = CreateComponent(SceneComponentType::TurntableCameraControl).AsStrong<TurntableCameraControl>();
	out->SetOrbitCenter(orbit);
	out->SetTableNormal(normal);

	return out;
}

StrongRef<Component> Scene::CreateComponent(core::Name type) const
{
	return core::ReferableFactory::Instance()->Create(type).AsStrong<Component>();
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
	return !GetRoot()->HasChildren();
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::RegisterAnimated(Node* node)
{
	m_AnimatedNodes.Insert(node);
}

void Scene::UnregisterAnimated(Node* node)
{
	m_AnimatedNodes.Erase(node);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::AnimateAll(float secsPassed)
{
	for(auto node : m_AnimatedNodes)
		node->Animate(secsPassed);
	ClearDeletionQueue();
}

////////////////////////////////////////////////////////////////////////////////////

static void VisitRenderablesRec(
	Node* node, RenderableVisitor* visitor,
	ERenderableTags tags)
{
	node->VisitRenderables(visitor, tags);

	for(auto child : node->Children())
		VisitRenderablesRec(child, visitor, tags);
}

void Scene::VisitRenderables(
	RenderableVisitor* visitor,
	ERenderableTags tags, Node* root)
{
	if(!root)
		root = m_Root;
	VisitRenderablesRec(root, visitor, tags);
}

////////////////////////////////////////////////////////////////////////////////////

void Scene::OnAttach(Node* node)
{
	for(auto& obs : m_Observers)
		obs->OnAttach(node);
}

void Scene::OnDetach(Node* node)
{
	for(auto& obs : m_Observers)
		obs->OnDetach(node);
}

void Scene::OnAttach(Component* comp)
{
	for(auto& obs : m_Observers)
		obs->OnAttach(comp);
}

void Scene::OnDetach(Component* comp)
{
	for(auto& obs : m_Observers)
		obs->OnDetach(comp);
}

} // namespace scene
} // namespace lux