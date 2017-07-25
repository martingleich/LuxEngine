#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H
#include "core/ReferenceCounted.h"

#include "scene/Renderable.h"

#include "core/lxName.h"

#include "math/vector3.h"
#include "math/quaternion.h"
#include "math/line3d.h"

#include "video/Color.h"

namespace lux
{
namespace math
{
class Transformation;
}
namespace video
{
class VideoDriver;
class Renderer;
class MaterialLibrary;
class ImageSystem;
class CubeTexture;
class PipelineSettings;
class PipelineOverwrite;
class FogData;
class Mesh;
class MeshSystem;
}

namespace scene
{
class Component;
class Node;
class Camera;
class Mesh;
class Light;
class SkyBox;

class Animator;
class CameraControl;
class RotationAnimator;
class LinearMoveAnimator;

class Collider;
class VolumeQuery;
class LineQuery;

class SceneManager : public ReferenceCounted
{
public:
	virtual ~SceneManager() {}

	virtual void Clear() = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual StrongRef<Node> AddNode(Component* baseComp=nullptr, Node* parent=nullptr) = 0;
	virtual StrongRef<Node> AddMesh(const io::Path& path) = 0;
	virtual StrongRef<Node> AddMesh(video::Mesh* mesh) = 0;
	virtual StrongRef<Node> AddSkyBox(video::CubeTexture* skyTexture=nullptr) = 0;
	virtual StrongRef<Node> AddLight() = 0;
	virtual StrongRef<Node> AddCamera() = 0;

	// Object components
	virtual StrongRef<Camera> CreateCamera() = 0;
	virtual StrongRef<Mesh> CreateMesh(const io::Path& path) = 0;
	virtual StrongRef<Mesh> CreateMesh(video::Mesh* mesh=nullptr) = 0;
	virtual StrongRef<SkyBox> CreateSkyBox(video::CubeTexture* skyTexture=nullptr) = 0;
	virtual StrongRef<Light> CreateLight() = 0;

	// Animatoren
	virtual StrongRef<RotationAnimator> CreateRotator(const math::vector3f& axis=math::vector3f::UNIT_Y, math::anglef rotSpeed = math::anglef::Degree(45.0f)) = 0;
	virtual StrongRef<LinearMoveAnimator> CreateLinearMover(const math::line3df& line, float duration) = 0;
	virtual StrongRef<CameraControl> CreateCameraControl(float moveSpeed=4.0f, math::anglef rotSpeed=math::anglef::Degree(9.0f), bool noVerticalMovement=false) = 0;

	virtual StrongRef<Component> CreateComponent(core::Name type) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual StrongRef<Collider> CreateMeshCollider(video::Mesh* mesh) = 0;
	virtual StrongRef<Collider> CreateBoundingBoxCollider() = 0;
	virtual StrongRef<Collider> CreateBoundingSphereCollider() = 0;
	virtual StrongRef<Collider> CreateBoxCollider(const math::vector3f& halfSize, const math::Transformation& trans) = 0;
	virtual StrongRef<Collider> CreateSphereCollider(const math::vector3f& center, float radius) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual Node* GetRoot() = 0;

	virtual StrongRef<Node> GetActiveCameraNode() = 0;
	virtual StrongRef<Camera> GetActiveCamera() = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual bool DrawAll(bool beginScene=true, bool endScene=true) = 0;
	virtual void AnimateAll(float secsPassed) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual void RegisterCamera(Node* node, Camera* camera) = 0;
	virtual void UnregisterCamera(Node* node, Camera* camera) = 0;

	virtual void RegisterLight(Node* node, Light* light) = 0;
	virtual void UnregisterLight(Node* node, Light* light) = 0;

	virtual void RegisterAnimated(Node* node) = 0;
	virtual void UnregisterAnimated(Node* node) = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual void AddToDeletionQueue(Node* node) = 0;
	virtual void ClearDeletionQueue() = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual void SetAmbient(const video::Colorf& ambient) = 0;
	virtual const video::Colorf& GetAmbient() const = 0;

	virtual void SetFog(const video::FogData& fog) = 0;
	virtual const video::FogData& GetFog() const = 0;

	////////////////////////////////////////////////////////////////////////////////////

	virtual void AddPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over) = 0;
	virtual void RemovePipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over) = 0;
};

} // namespace scene
} // namespace lux

#endif
