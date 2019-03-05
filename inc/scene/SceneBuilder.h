#ifndef INCLUDED_LUX_SCENE_BUILDER_H
#define INCLUDED_LUX_SCENE_BUILDER_H
#include "scene/Scene.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Line3.h"

#include "video/Color.h"

#include "io/Path.h"

namespace lux
{
namespace math
{
class Transformation;
}
namespace video
{
class CubeTexture;
class Mesh;
}
namespace scene
{
class PerspCamera;
class Mesh;
class DirectionalLight;
class PointLight;
class SpotLight;
class LinearFog;
class ExponentialFog;
class SkyBox;

class Animator;
class FirstPersonCameraControl;
class TurntableCameraControl;
class RotationAnimator;
class LinearMoveAnimator;

class Collider;
class VolumeQuery;
class LineQuery;

class SceneBuilder
{
public:
	LUX_API SceneBuilder(Scene* scene, Node* defaultRoot = nullptr);

	LUX_API StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);
	LUX_API StrongRef<Node> AddMesh(const io::Path& path);
	LUX_API StrongRef<Node> AddMesh(video::Mesh* mesh);
	LUX_API StrongRef<Node> AddSkyBox(const video::ColorF& color);
	LUX_API StrongRef<Node> AddSkyBox(video::CubeTexture* skyTexture = nullptr);
	LUX_API StrongRef<Node> AddDirectionalLight(video::ColorF color = video::Color::White);
	LUX_API StrongRef<Node> AddPointLight(video::ColorF color = video::Color::White);
	LUX_API StrongRef<Node> AddSpotLight(video::ColorF color = video::Color::White);
	LUX_API StrongRef<Node> AddLinearFog(float start, float end, const video::ColorF& color = video::Color::White);
	LUX_API StrongRef<Node> AddExponentialFog(float density, const video::ColorF& color = video::Color::White);
	LUX_API StrongRef<Node> AddCamera(float aspectRatio);

	// Object components
	LUX_API StrongRef<PerspCamera> CreateCamera(float aspectRatio) const;
	LUX_API StrongRef<Mesh> CreateMesh(const io::Path& path) const;
	LUX_API StrongRef<Mesh> CreateMesh(video::Mesh* mesh = nullptr) const;
	LUX_API StrongRef<SkyBox> CreateSkyBox(const video::ColorF& color) const;
	LUX_API StrongRef<SkyBox> CreateSkyBox(video::CubeTexture* skyTexture = nullptr) const;
	LUX_API StrongRef<DirectionalLight> CreateDirectionalLight(video::ColorF color = video::Color::White) const;
	LUX_API StrongRef<PointLight> CreatePointLight(video::ColorF color = video::Color::White) const;
	LUX_API StrongRef<SpotLight> CreateSpotLight(video::ColorF color = video::Color::White) const;
	LUX_API StrongRef<LinearFog> CreateLinearFog(float start, float end, const video::ColorF& color = video::Color::White);
	LUX_API StrongRef<ExponentialFog> CreateExponentialFog(float density, const video::ColorF& color = video::Color::White);

	// Animatoren
	LUX_API StrongRef<RotationAnimator> CreateRotator(const math::Vector3F& axis = math::Vector3F::UNIT_Y, math::AngleF rotSpeed = math::AngleF::Degree(45.0f)) const;
	LUX_API StrongRef<LinearMoveAnimator> CreateLinearMover(const math::Line3F& line, float duration) const;
	LUX_API StrongRef<FirstPersonCameraControl> CreateFirstPersonCameraControl(float moveSpeed = 4.0f, math::AngleF rotSpeed = math::AngleF::Degree(9.0f), bool noVerticalMovement = false) const;
	LUX_API StrongRef<TurntableCameraControl> CreateTurntableCameraControl(const math::Vector3F& orbit = math::Vector3F::ZERO, const math::Vector3F& normal = math::Vector3F::UNIT_Y) const;

	LUX_API StrongRef<Component> CreateComponent(core::Name type) const;

	LUX_API StrongRef<Collider> CreateMeshCollider(video::Mesh* mesh) const;
	LUX_API StrongRef<Collider> CreateBoundingBoxCollider() const;
	LUX_API StrongRef<Collider> CreateBoundingSphereCollider() const;
	LUX_API StrongRef<Collider> CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) const;
	LUX_API StrongRef<Collider> CreateSphereCollider(const math::Vector3F& center, float radius) const;

protected:
	Scene* m_Scene;
	Node* m_DefaultRoot;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_BUILDER_H