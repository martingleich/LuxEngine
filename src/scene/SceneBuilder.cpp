#include "scene/SceneBuilder.h"

#include "scene/Node.h"

#include "core/ReferableFactory.h"

#include "core/ResourceSystem.h"

#include "video/mesh/VideoMesh.h"

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

///////////////////////////////////////////////////////////////////////////////

StrongRef<Node> SceneBuilder::AddNode(Component* baseComp, Node* parent)
{
	return m_Scene->AddNode(baseComp, parent);
}

StrongRef<Node> SceneBuilder::AddMesh(const io::Path& path)
{
	return m_Scene->AddNode(CreateMesh(path));
}

StrongRef<Node> SceneBuilder::AddMesh(video::Mesh* mesh)
{
	return m_Scene->AddNode(CreateMesh(mesh));
}

StrongRef<Node> SceneBuilder::AddSkyBox(const video::ColorF& color)
{
	return m_Scene->AddNode(CreateSkyBox(color));
}

StrongRef<Node> SceneBuilder::AddSkyBox(video::CubeTexture* skyTexture)
{
	return m_Scene->AddNode(CreateSkyBox(skyTexture));
}

StrongRef<Node> SceneBuilder::AddDirectionalLight(video::Color color)
{
	return m_Scene->AddNode(CreateDirectionalLight(color));
}
StrongRef<Node> SceneBuilder::AddPointLight(video::Color color)
{
	return m_Scene->AddNode(CreatePointLight(color));
}
StrongRef<Node> SceneBuilder::AddSpotLight(video::Color color)
{
	return m_Scene->AddNode(CreateSpotLight(color));
}

StrongRef<Node> SceneBuilder::AddLinearFog(float start, float end, const video::ColorF& color)
{
	return m_Scene->AddNode(CreateLinearFog(start, end, color));
}

StrongRef<Node> SceneBuilder::AddExponentialFog(float density, const video::ColorF& color)
{
	return m_Scene->AddNode(CreateExponentialFog(density, color));
}

StrongRef<Node> SceneBuilder::AddCamera(float aspect)
{
	return m_Scene->AddNode(CreateCamera(aspect));
}

StrongRef<Camera> SceneBuilder::CreateCamera(float aspect) const
{
	auto cam = CreateComponent(SceneComponentType::Camera).AsStrong<Camera>();
	cam->SetAspect(aspect);
	return cam;
}

StrongRef<Mesh> SceneBuilder::CreateMesh(const io::Path& path) const
{
	return CreateMesh(core::ResourceSystem::Instance()->GetResource(core::ResourceType::Mesh, path.AsView()).As<video::Mesh>());
}

StrongRef<Mesh> SceneBuilder::CreateMesh(video::Mesh* mesh) const
{
	auto out = CreateComponent(SceneComponentType::Mesh).AsStrong<Mesh>();
	out->SetMesh(mesh);

	return out;
}

StrongRef<SkyBox> SceneBuilder::CreateSkyBox(const video::ColorF& color) const
{
	auto out = CreateComponent(SceneComponentType::SkyBox).AsStrong<SkyBox>();
	out->GetMaterial()->SetDiffuse(color);
	return out;
}

StrongRef<SkyBox> SceneBuilder::CreateSkyBox(video::CubeTexture* skyTexture) const
{
	auto out = CreateComponent(SceneComponentType::SkyBox).AsStrong<SkyBox>();
	out->SetSkyTexture(skyTexture);

	return out;
}

StrongRef<DirectionalLight> SceneBuilder::CreateDirectionalLight(video::ColorF color) const
{
	auto light = CreateComponent(SceneComponentType::DirLight).AsStrong<DirectionalLight>();
	light->SetColor(color);
	return light;
}

StrongRef<PointLight> SceneBuilder::CreatePointLight(video::ColorF color) const
{
	auto light = CreateComponent(SceneComponentType::PointLight).AsStrong<PointLight>();
	light->SetColor(color);
	return light;
}

StrongRef<SpotLight> SceneBuilder::CreateSpotLight(video::ColorF color) const
{
	auto light = CreateComponent(SceneComponentType::SpotLight).AsStrong<SpotLight>();
	light->SetColor(color);
	return light;
}

StrongRef<LinearFog> SceneBuilder::CreateLinearFog(float start, float end, const video::ColorF& color)
{
	auto fog = CreateComponent(SceneComponentType::LinearFog).AsStrong<LinearFog>();
	fog->SetStart(start);
	fog->SetEnd(end);
	fog->SetColor(color);

	return fog;
}

StrongRef<ExponentialFog> SceneBuilder::CreateExponentialFog(float density, const video::ColorF& color)
{
	auto fog = CreateComponent(SceneComponentType::ExponentialFog).AsStrong<ExponentialFog>();
	fog->SetDensity(density);
	fog->SetColor(color);

	return fog;
}


StrongRef<RotationAnimator> SceneBuilder::CreateRotator(const math::Vector3F& axis, math::AngleF rotSpeed) const
{
	auto out = CreateComponent(SceneComponentType::Rotation).AsStrong<RotationAnimator>();
	out->SetAxis(axis);
	out->SetRotationSpeed(rotSpeed);

	return out;
}

StrongRef<LinearMoveAnimator> SceneBuilder::CreateLinearMover(const math::Line3F& line, float duration) const
{
	auto out = CreateComponent(SceneComponentType::LinearMove).AsStrong<LinearMoveAnimator>();
	out->SetData(line, duration);

	return out;
}

StrongRef<FirstPersonCameraControl> SceneBuilder::CreateFirstPersonCameraControl(float moveSpeed, math::AngleF rotSpeed, bool noVerticalMovement) const
{
	auto out = CreateComponent(SceneComponentType::FirstPersonCameraControl).AsStrong<FirstPersonCameraControl>();
	out->SetMoveSpeed(moveSpeed);
	out->SetRotationSpeed(rotSpeed);
	out->AllowVerticalMovement(!noVerticalMovement);

	return out;
}

StrongRef<TurntableCameraControl> SceneBuilder::CreateTurntableCameraControl(const math::Vector3F& orbit, const math::Vector3F& normal) const
{
	auto out = CreateComponent(SceneComponentType::TurntableCameraControl).AsStrong<TurntableCameraControl>();
	out->SetOrbitCenter(orbit);
	out->SetTableNormal(normal);

	return out;
}

StrongRef<Component> SceneBuilder::CreateComponent(core::Name type) const
{
	return core::ReferableFactory::Instance()->Create(type).AsStrong<Component>();
}

////////////////////////////////////////////////////////////////////////////////////

StrongRef<Collider> SceneBuilder::CreateMeshCollider(video::Mesh* mesh) const
{
	return LUX_NEW(MeshCollider)(mesh);
}

StrongRef<Collider> SceneBuilder::CreateBoundingBoxCollider() const
{
	return LUX_NEW(BoundingBoxCollider)();
}

StrongRef<Collider> SceneBuilder::CreateBoundingSphereCollider() const
{
	return LUX_NEW(BoundingSphereCollider)();
}

StrongRef<Collider> SceneBuilder::CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) const
{
	return LUX_NEW(BoxCollider)(halfSize, trans);
}

StrongRef<Collider> SceneBuilder::CreateSphereCollider(const math::Vector3F& center, float radius) const
{
	return LUX_NEW(SphereCollider)(center, radius);
}

} // namespace scene
} // namespace lux
