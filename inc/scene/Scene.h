#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/lxOrderedSet.h"
#include "core/lxName.h"

#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Line3.h"

#include "video/Color.h"
#include "video/LightData.h"
#include "video/FogData.h"

#include "scene/Renderable.h"

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
class Component;
class Node;
class Camera;
class Mesh;
class Light;
class GlobalFog;
class SkyBox;

class Animator;
class FirstPersonCameraControl;
class TurntableCameraControl;
class RotationAnimator;
class LinearMoveAnimator;

class Collider;
class VolumeQuery;
class LineQuery;

class SceneObserver : public ReferenceCounted
{
public:
	// Only called for parent node
	virtual void OnAttach(Node* child) = 0;
	// Only called for parent node
	virtual void OnDetach(Node* child) = 0;

	// Only if component is attached to a node.
	// Not if a node with components is attached to a node in the scene.
	virtual void OnAttach(Component* child) = 0;
	// Only if component is detached to a node.
	// Not if a node with components is detached to a node in the scene.
	virtual void OnDetach(Component* child) = 0;
};

class Scene : public ReferenceCounted
{
public:
	LUX_API Scene();
	Scene(const Scene&) = delete;
	LUX_API ~Scene();

	LUX_API void RegisterObserver(SceneObserver* observer);
	LUX_API void UnregisterObserver(SceneObserver* observer);

	LUX_API void Clear();
	LUX_API void AddToDeletionQueue(Node* node);
	LUX_API void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);
	LUX_API StrongRef<Node> AddMesh(const io::Path& path);
	LUX_API StrongRef<Node> AddMesh(video::Mesh* mesh);
	LUX_API StrongRef<Node> AddSkyBox(const video::ColorF& color);
	LUX_API StrongRef<Node> AddSkyBox(video::CubeTexture* skyTexture = nullptr);
	LUX_API StrongRef<Node> AddLight(video::ELightType lightType = video::ELightType::Point, video::Color color = video::Color::White);
	LUX_API StrongRef<Node> AddFog(const video::ColorF& color = video::Color::White, float start = 10.0f, float end = 100.0f);
	LUX_API StrongRef<Node> AddCamera();

	// Object components
	LUX_API StrongRef<Camera> CreateCamera() const;
	LUX_API StrongRef<Mesh> CreateMesh(const io::Path& path) const;
	LUX_API StrongRef<Mesh> CreateMesh(video::Mesh* mesh = nullptr) const;
	LUX_API StrongRef<SkyBox> CreateSkyBox(const video::ColorF& color) const;
	LUX_API StrongRef<SkyBox> CreateSkyBox(video::CubeTexture* skyTexture = nullptr) const;
	LUX_API StrongRef<Light> CreateLight(video::ELightType lightType = video::ELightType::Point, video::Color color = video::Color::White) const;
	LUX_API StrongRef<GlobalFog> CreateFog(const video::ColorF& color = video::Color::White, float start = 10.0f, float end = 100.0f) const;

	// Animatoren
	LUX_API StrongRef<RotationAnimator> CreateRotator(const math::Vector3F& axis = math::Vector3F::UNIT_Y, math::AngleF rotSpeed = math::AngleF::Degree(45.0f)) const;
	LUX_API StrongRef<LinearMoveAnimator> CreateLinearMover(const math::Line3F& line, float duration) const;
	LUX_API StrongRef<FirstPersonCameraControl> CreateFirstPersonCameraControl(float moveSpeed = 4.0f, math::AngleF rotSpeed = math::AngleF::Degree(9.0f), bool noVerticalMovement = false) const;
	LUX_API StrongRef<TurntableCameraControl> CreateTurntableCameraControl(const math::Vector3F& orbit = math::Vector3F::ZERO, const math::Vector3F& normal = math::Vector3F::UNIT_Y) const;

	LUX_API StrongRef<Component> CreateComponent(core::Name type) const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Collider> CreateMeshCollider(video::Mesh* mesh) const;
	LUX_API StrongRef<Collider> CreateBoundingBoxCollider() const;
	LUX_API StrongRef<Collider> CreateBoundingSphereCollider() const;
	LUX_API StrongRef<Collider> CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) const;
	LUX_API StrongRef<Collider> CreateSphereCollider(const math::Vector3F& center, float radius) const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API Node* GetRoot() const;
	LUX_API bool IsEmpty() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void RegisterAnimated(Node* node);
	LUX_API void UnregisterAnimated(Node* node);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void AnimateAll(float secsPassed);
	LUX_API void VisitRenderables(
		RenderableVisitor* visitor,
		ERenderableTags tags = ERenderableTags::None, Node* root = nullptr);

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API void OnAttach(Node* node);
	LUX_API void OnDetach(Node* node);

	LUX_API void OnAttach(Component* comp);
	LUX_API void OnDetach(Component* comp);

private:
	StrongRef<Node> m_Root; //!< The root of the scenegraph

	core::Array<StrongRef<SceneObserver>> m_Observers;
	core::Array<StrongRef<Node>> m_DeletionQueue; //!< Nodes to delete on next deletion run

	core::OrderedSet<Node*> m_AnimatedNodes; //!< The animated nodes of the graph
};

} // namespace scene
} // namespace lux

#endif
