#ifndef INCLUDED_SCENEMANAGER_IMPL_H
#define INCLUDED_SCENEMANAGER_IMPL_H
#include "scene/SceneManager.h"
#include "video/FogData.h"
#include "scene/components/Camera.h"
#include "scene/Node.h"

namespace lux
{
namespace scene
{

class SceneManagerImpl : public SceneManager
{
public:
	SceneManagerImpl(
		video::VideoDriver* driver,
		video::ImageSystem* imageSystem,
		io::FileSystem* fileSystem,
		core::ReferableFactory* refFactory,
		video::MeshSystem* meshCache,
		core::ResourceSystem* resourceSystem,
		video::MaterialLibrary* matLib);

	~SceneManagerImpl();

	void Clear();

	////////////////////////////////////////////////////////////////////////////////////

	StrongRef<Node> AddNode(Component* baseComp=nullptr, Node* parent=nullptr);

	// Object components
	StrongRef<Camera> CreateCamera();
	StrongRef<Mesh> CreateMesh(const io::path& path);
	StrongRef<Mesh> CreateMesh(video::Mesh* mesh=nullptr);
	StrongRef<SkyBox> CreateSkyBox(video::CubeTexture* skyTexture=nullptr);
	StrongRef<Light> CreateLight();

	// Animatoren
	StrongRef<RotationAnimator> CreateRotator(const math::vector3f& axis=math::vector3f::UNIT_Y, math::anglef rotSpeed = math::anglef::Degree(45.0f));
	StrongRef<LinearMoveAnimator> CreateLinearMover(const math::line3df& line, float duration);
	StrongRef<CameraControl> CreateCameraControl(float moveSpeed=4.0f, math::anglef rotSpeed=math::anglef::Degree(90.0f), bool noVerticalMovement=false);

	StrongRef<Component> CreateComponent(core::Name type);

	////////////////////////////////////////////////////////////////////////////////////

	StrongRef<Collider> CreateMeshCollider(video::Mesh* mesh);
	StrongRef<Collider> CreateBoundingBoxCollider();
	StrongRef<Collider> CreateBoundingSphereCollider();
	StrongRef<Collider> CreateBoxCollider(const math::vector3f& halfSize, const math::Transformation& trans);
	StrongRef<Collider> CreateSphereCollider(const math::vector3f& center, float radius);

	////////////////////////////////////////////////////////////////////////////////////

	Node* GetRoot();

	StrongRef<Node> GetActiveCameraNode();
	StrongRef<Camera> GetActiveCamera();

	////////////////////////////////////////////////////////////////////////////////////

	bool DrawAll(bool beginScene=true, bool endScene=true);
	void AnimateAll(float secsPassed);

	////////////////////////////////////////////////////////////////////////////////////

	void RegisterCamera(Node* node, Camera* camera);
	void UnregisterCamera(Node* node, Camera* camera);

	void RegisterLight(Node* node, Light* light);
	void UnregisterLight(Node* node, Light* light);

	void RegisterEventReceiver(input::EventReceiver* receiver);
	void UnregisterEventReceiver(input::EventReceiver* receiver);

	////////////////////////////////////////////////////////////////////////////////////

	void AddToDeletionQueue(Node* node);
	void ClearDeletionQueue();

	////////////////////////////////////////////////////////////////////////////////////

	void SetAmbient(const video::Colorf& ambient);
	const video::Colorf& GetAmbient() const;

	void SetFog(const video::FogData& fog);
	const video::FogData& GetFog() const;

	////////////////////////////////////////////////////////////////////////////////////

	void AddPipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over);
	void RemovePipelineOverwrite(ERenderPass pass, const video::PipelineOverwrite& over);

	////////////////////////////////////////////////////////////////////////////////////

	video::VideoDriver* GetDriver() const;
	video::Renderer* GetRenderer() const;
	io::FileSystem* GetFileSystem() const;
	video::MaterialLibrary* GetMaterialLibrary() const;
	video::ImageSystem* GetImageSystem() const;
	core::ReferableFactory* GetReferableFactory() const;
	core::ResourceSystem* GetResourceSystem() const;
	video::MeshSystem* GetMeshSystem() const;

private:
	bool OnEvent(const input::Event& e);
	void EnableOverwrite(ERenderPass pass);
	void DisableOverwrite(ERenderPass pass);

	size_t GetPassId(ERenderPass pass) const;

	class RenderableCollector;
	void CollectRenderables(Node* root);
	void CollectRenderablesRec(Node* node, RenderableCollector* collector, bool noDebug);
	void AddRenderEntry(Node* n, Renderable* r);
	void DrawScene();

private:
	struct RenderEntry
	{
	public:
		Node* node;
		Renderable* renderable;

		RenderEntry() :
			node(nullptr),
			renderable(nullptr)
		{}

		RenderEntry(Node* n, Renderable* r) :
			node(n),
			renderable(r)
		{}
	};

	struct DistanceRenderEntry : public RenderEntry
	{
		float distance;
		math::vector3f pos;

		DistanceRenderEntry()
		{}

		DistanceRenderEntry(Node* n, Renderable* r) :
			RenderEntry(n, r)
		{
			pos = node->GetAbsolutePosition();
		}

		void UpdateDistance(const math::vector3f& camera)
		{
			distance = pos.GetDistanceToSq(camera);
		}
	
		bool operator<(const DistanceRenderEntry& other) const
		{
			// The farthest element must be first in list
			return distance > other.distance;
		}
	};

	struct CameraEntry
	{
		Node* node;
		Camera* camera;

		CameraEntry() :
			node(nullptr),
			camera(nullptr)
		{}
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
		{}

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

private:
	/////////////////////////////////////////////////////////////////////////
	// Caches and temporary values
	/////////////////////////////////////////////////////////////////////////
	// The used renderqueues
	core::array<CameraEntry> m_CameraList;
	core::array<LightEntry> m_LightList;
	core::array<RenderEntry> m_SkyBoxList;
	core::array<RenderEntry> m_SolidNodeList;
	core::array<DistanceRenderEntry> m_TransparentNodeList;
	Node* m_CollectedRoot;

	// Information about the current camera
	WeakRef<Node> m_ActiveCameraNode;
	WeakRef<Camera> m_ActiveCamera;
	math::vector3f m_AbsoluteCamPos;

	core::array<StrongRef<Node>> m_DeletionQueue; //!< Nodes to delete on next deletion run
	core::array<input::EventReceiver*> m_EventReceivers; //!< List of event receiving nodes

	/////////////////////////////////////////////////////////////////////////
	// Settings and parameters
	/////////////////////////////////////////////////////////////////////////

	// Scene values
	video::Colorf m_AmbientColor;
	video::FogData m_Fog;

	core::array<core::array<video::PipelineOverwrite>> m_Overwrites; //!< User-set pipeline overwrites

	StrongRef<Node> m_RootSceneNode; //!< The root of the scenegraph

	/////////////////////////////////////////////////////////////////////////
	// References to other classes
	/////////////////////////////////////////////////////////////////////////

	StrongRef<video::VideoDriver> m_Driver;
	video::Renderer* m_Renderer;

	StrongRef<io::FileSystem> m_Filesystem;
	StrongRef<video::MeshSystem> m_MeshSystem;
	StrongRef<core::ReferableFactory> m_RefFactory;
	StrongRef<video::ImageSystem> m_ImagSys;
	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<video::MaterialLibrary> m_MatLib;
};

} // namespace scene
} // namespace lux

#endif