#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H
#include "core/ReferenceCounted.h"

#include "core/lxOrderedSet.h"
#include "core/lxArray.h"

#include "core/lxName.h"

#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Line3.h"

#include "video/Color.h"

#include "video/LightData.h"
#include "video/FogData.h"

#include "scene/components/Camera.h"

#include "scene/Node.h"

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
class Fog;
class SkyBox;

class Animator;
class FirstPersonCameraControl;
class RotationAnimator;
class LinearMoveAnimator;

class Collider;
class VolumeQuery;
class LineQuery;

class Scene : public ReferenceCounted
{
public:
	struct CameraEntry
	{
		Node* node;
		Camera* camera;

		CameraEntry() :
			node(nullptr),
			camera(nullptr)
		{
		}
		CameraEntry(Node* n, Camera* c) :
			node(n),
			camera(c)
		{
		}

		bool operator<(const CameraEntry& other) const
		{
			return camera->GetRenderPriority() > other.camera->GetRenderPriority();
		}
		bool operator==(const CameraEntry& other) const
		{
			return (node == other.node && camera == other.camera);
		}
	};

	struct LightEntry
	{
		Node* node;
		Light* light;

		LightEntry() :
			node(nullptr),
			light(nullptr)
		{
		}

		LightEntry(Node* n, Light* l) :
			node(n),
			light(l)
		{
		}

		bool operator==(const LightEntry& other) const
		{
			return node == other.node && light == other.light;
		}
	};

	struct FogEntry
	{
		Node* node;
		Fog* fog;

		FogEntry() :
			node(nullptr),
			fog(nullptr)
		{
		}

		FogEntry(Node* n, Fog* f) :
			node(n),
			fog(f)
		{
		}

		bool operator==(const FogEntry& other) const
		{
			return node == other.node && fog == other.fog;
		}
	};

public:
	LUX_API Scene();
	LUX_API virtual ~Scene();

	LUX_API virtual void Clear();
	LUX_API virtual void AddToDeletionQueue(Node* node);
	LUX_API virtual void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual StrongRef<Node> AddNode(Component* baseComp = nullptr, Node* parent = nullptr);
	LUX_API virtual StrongRef<Node> AddMesh(const io::Path& path);
	LUX_API virtual StrongRef<Node> AddMesh(video::Mesh* mesh);
	LUX_API virtual StrongRef<Node> AddSkyBox(const video::Colorf& color);
	LUX_API virtual StrongRef<Node> AddSkyBox(video::CubeTexture* skyTexture = nullptr);
	LUX_API virtual StrongRef<Node> AddLight(video::ELightType lightType = video::ELightType::Point, video::Color color = video::Color::White);
	LUX_API virtual StrongRef<Node> AddFog(const video::Colorf& color = video::Color::White, float start = 10.0f, float end = 100.0f);
	LUX_API virtual StrongRef<Node> AddCamera();

	// Object components
	LUX_API virtual StrongRef<Camera> CreateCamera() const;
	LUX_API virtual StrongRef<Mesh> CreateMesh(const io::Path& path) const;
	LUX_API virtual StrongRef<Mesh> CreateMesh(video::Mesh* mesh = nullptr) const;
	LUX_API virtual StrongRef<SkyBox> CreateSkyBox(video::CubeTexture* skyTexture = nullptr) const;
	LUX_API virtual StrongRef<Light> CreateLight(video::ELightType lightType = video::ELightType::Point, video::Color color = video::Color::White) const;
	LUX_API virtual StrongRef<Fog> CreateFog(const video::Colorf& color = video::Color::White, float start = 10.0f, float end = 100.0f) const;

	// Animatoren
	LUX_API virtual StrongRef<RotationAnimator> CreateRotator(const math::Vector3F& axis = math::Vector3F::UNIT_Y, math::AngleF rotSpeed = math::AngleF::Degree(45.0f)) const;
	LUX_API virtual StrongRef<LinearMoveAnimator> CreateLinearMover(const math::Line3F& line, float duration) const;
	LUX_API virtual StrongRef<FirstPersonCameraControl> CreateFirstPersonCameraControl(float moveSpeed = 4.0f, math::AngleF rotSpeed = math::AngleF::Degree(9.0f), bool noVerticalMovement = false) const;

	LUX_API virtual StrongRef<Component> CreateComponent(core::Name type) const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual StrongRef<Collider> CreateMeshCollider(video::Mesh* mesh) const;
	LUX_API virtual StrongRef<Collider> CreateBoundingBoxCollider() const;
	LUX_API virtual StrongRef<Collider> CreateBoundingSphereCollider() const;
	LUX_API virtual StrongRef<Collider> CreateBoxCollider(const math::Vector3F& halfSize, const math::Transformation& trans) const;
	LUX_API virtual StrongRef<Collider> CreateSphereCollider(const math::Vector3F& center, float radius) const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual Node* GetRoot() const;
	LUX_API virtual bool IsEmpty() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual void RegisterCamera(Node* node, Camera* camera);
	LUX_API virtual void UnregisterCamera(Node* node, Camera* camera);

	LUX_API virtual void RegisterLight(Node* node, Light* light);
	LUX_API virtual void UnregisterLight(Node* node, Light* light);

	LUX_API virtual void RegisterFog(Node* node, Fog* fog);
	LUX_API virtual void UnregisterFog(Node* node, Fog* fog);

	LUX_API virtual void RegisterAnimated(Node* node);
	LUX_API virtual void UnregisterAnimated(Node* node);

	LUX_API virtual const core::Array<CameraEntry>& GetCameraList() const;
	LUX_API virtual const core::Array<LightEntry>& GetLightList() const;
	LUX_API virtual const core::Array<FogEntry>& GetFogList() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual void SetAmbient(const video::Colorf& ambient);
	LUX_API virtual const video::Colorf& GetAmbient() const;

	////////////////////////////////////////////////////////////////////////////////////

	LUX_API virtual void AnimateAll(float secsPassed);

private:
	StrongRef<Node> m_Root; //!< The root of the scenegraph

	video::Colorf m_AmbientColor;

	core::Array<StrongRef<Node>> m_DeletionQueue; //!< Nodes to delete on next deletion run

	core::OrderedSet<Node*> m_AnimatedNodes; //!< The animated nodes of the graph
	core::Array<CameraEntry> m_CameraList; //!< The cameras of the graph
	core::Array<LightEntry> m_LightList; //!< The lights of the graph
	core::Array<FogEntry> m_FogList; //!< The fogs of the graph
};

} // namespace scene
} // namespace lux

#endif
