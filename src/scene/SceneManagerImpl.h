#ifndef INCLUDED_SCENEMANAGER_IMPL_H
#define INCLUDED_SCENEMANAGER_IMPL_H
#include "scene/SceneManager.h"

namespace lux
{
namespace video
{
class VideoDriver;
}
namespace core
{
class ResourceSystem;
class ReferableFactory;
}
namespace scene
{
class MeshCache;

// Der Scenemanager kontrolliert, die Beleuchtung, Kameras, Renderreihenfolge, das Laden vom Modellen für eine Szene
class SceneManagerImpl : public SceneManager
{
public:
	// Konstruktor
	SceneManagerImpl(video::VideoDriver* driver,
		video::ImageSystem* imageSystem,
		io::FileSystem* fileSystem,
		core::ReferableFactory* refFactory,
		MeshCache* meshCache,
		core::ResourceSystem* resourceSystem,
		video::MaterialLibrary* matLib);

	// Destruktor
	virtual ~SceneManagerImpl();

	//--------------------------------------------------------------------------
	// Scene-Nodes

	// Fügt eine neue Kamera zum Scene-Graph hinzu
	// Rückgabe muss nicht gedropt werden
	virtual StrongRef<CameraSceneNode> AddCameraSceneNode(const math::vector3f& position = math::vector3f::ZERO,
		const math::vector3f& direction = math::vector3f::UNIT_Z,
		bool makeActive = true);

	// Fügt ein neues Modell zum Scene-Graph hinzu
	virtual StrongRef<MeshSceneNode> AddMeshSceneNode(Mesh* mesh,
		bool addIfMeshIsEmpty = true,
		const math::vector3f& position = math::vector3f::ZERO,
		const math::quaternionf& orient = math::quaternionf(0.0f, 0.0f, 0.0f, 1.0f),
		float scale = 1.0f);

	// Fügt eine Sky-box zum Scene-Graph hinzu
	virtual StrongRef<SceneNode> AddSkyBoxSceneNode(video::CubeTexture* skyTexture);

	virtual StrongRef<LightSceneNode> AddLightSceneNode(const math::vector3f position = math::vector3f::ZERO,
		const video::Colorf color = video::Colorf(1.0f, 1.0f, 1.0f),
		float range = 100.0f);

	virtual StrongRef<SceneNode> AddEmptySceneNode(SceneNode* parent = nullptr);

	virtual StrongRef<SceneNode> AddSceneNode(core::Name type, SceneNode* parent = nullptr);
	virtual StrongRef<SceneNodeComponent> AddComponent(core::Name type, SceneNode* node);

	//--------------------------------------------------------------------------

	virtual StrongRef<SceneNodeComponent> AddRotationAnimator(SceneNode* addTo, const math::vector3f& axis, math::anglef rotSpeed);

	virtual StrongRef<CameraFPSAnimator> AddCameraFPSAnimator(CameraSceneNode* addTo, float moveSpeed, math::anglef rotSpeed,
		bool noVerticalMovement = false,
		math::anglef maxVerticalAngle = math::anglef::Degree(89.0f));

	virtual StrongRef<SceneNodeComponent> AddLinearMoveAnimator(SceneNode* addTo,
		const math::line3df& line,
		float duration,
		bool jumpBack = false,
		u32 count = std::numeric_limits<u32>::max());

	//--------------------------------------------------------------------------

	video::VideoDriver* GetDriver();
	io::FileSystem* GetFileSystem();
	video::MaterialLibrary* GetMaterialLibrary();
	video::ImageSystem* GetImageSystem() const;
	virtual MeshCache* GetMeshCache();
	virtual core::ReferableFactory* GetReferableFactory() const;
	virtual core::ResourceSystem* GetResourceSystem() const;
	virtual SceneNode* GetRootSceneNode();
	virtual SceneNode* GetSceneNodeByType(core::Name type, SceneNode* start = nullptr, u32 tags=0);
	virtual bool GetSceneNodeArrayByType(core::Name type, core::array<SceneNode*>& array, SceneNode* start = nullptr, u32 tags=0);
	virtual StrongRef<CameraSceneNode> GetActiveCamera();
	virtual void SetActiveCamera(CameraSceneNode* camera);
	virtual void Render();
	virtual void DrawAll();
	virtual void AnimateAll(float secsPassed);
	virtual bool RegisterNodeForRendering(SceneNode* node, ESceneNodeRenderPass renderPass);
	virtual void AddToDeletionQueue(SceneNode* node);
	virtual void Clear();
	virtual void SetAmbient(video::Color ambient);
	virtual video::Color GetAmbient();
	virtual ESceneNodeRenderPass GetActRenderPass() const;
	void RegisterEventReceiver(SceneNode* receiver);
	void UnregisterEventReceiver(SceneNode* receiver);

	bool OnEvent(const input::Event& event);

	void AddPipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over);

	void RemovePipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over);

	void EnableOverwrite();

	void DisableOverwrite();

private:
	// Leert die Deletionqueue
	virtual void ClearDeletionQueue();

private:
	// Sortiert nach Textur
	struct SSolidNodeEntry
	{
	private:
		void* texture;
	public:
		SceneNode* node;

		SSolidNodeEntry();
		SSolidNodeEntry(SceneNode* node);
		bool operator<(const SSolidNodeEntry& other) const;
		bool operator==(const SSolidNodeEntry& other) const;
	};

	// Sortiert nach Abstand zur Kamera
	struct STransparentNodeEntry
	{
	private:
		double distance;
	public:
		SceneNode* node;

		STransparentNodeEntry();
		STransparentNodeEntry(SceneNode* node, math::vector3f vCameraPos);
		bool operator<(const STransparentNodeEntry& other) const;
		bool operator==(const STransparentNodeEntry& other) const;
	};

private:
	StrongRef<video::VideoDriver> m_Driver;
	StrongRef<io::FileSystem> m_Filesystem;
	StrongRef<MeshCache> m_MeshCache;
	StrongRef<core::ReferableFactory> m_RefFactory;
	StrongRef<video::ImageSystem> m_ImagSys;
	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<video::MaterialLibrary> m_MatLib;

	//-------------------------------------------------------------

	// Die verschieden Gruppen von Scene-Nodes
	core::array<CameraSceneNode*> m_CameraList;
	core::array<LightSceneNode*> m_LightList;
	core::array<SceneNode*> m_SkyBoxList;
	core::array<SSolidNodeEntry> m_SolidNodeList;
	core::array<STransparentNodeEntry> m_TransparentNodeList;

	core::array<WeakRef<SceneNode>> m_EventReceivers;

	core::array<core::array<video::PipelineOverwrite>> m_Overwrites;

	WeakRef<CameraSceneNode> m_ActiveCamera;
	math::vector3f m_AbsoluteCamPos;

	// Der Renderpass der gerade gezeichnet wird
	ESceneNodeRenderPass m_CurrentRenderPass;

	// Das Hintergrundlicht
	video::Color m_AmbientColor;

	// Die DeletionList, alle zu löschenden Knoten sollten ihr hinzugefügt werden, sie werden dann zu einen Zeitpunkt
	// gelöscht an dem es zu keinen Komplikationen kommen kann
	core::array<StrongRef<SceneNode>> m_DeletionList;

	StrongRef<SceneNode> m_RootSceneNode;

};

}    

}    


#endif