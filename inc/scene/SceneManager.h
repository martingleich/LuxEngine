#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H
#include "math/vector3.h"
#include "math/quaternion.h"
#include "math/line3d.h"

#include "core/lxName.h"
#include "core/lxArray.h"
#include "core/ReferableFactory.h"

#include "video/Color.h"
#include "video/PipelineSettings.h"

#include "input/EventReceiver.h"

#include "resources/ResourceSystem.h"

namespace lux
{
namespace video
{
class VideoDriver;
class MaterialLibrary;
class ImageSystem;
class CubeTexture;
}

namespace io
{
class FileSystem;
class File;
}

namespace scene
{

class MeshCache;
class Mesh;

class SceneNode;
class CameraSceneNode;
class MeshSceneNode;
class LightSceneNode;

class SceneNodeComponent;
class CameraFPSAnimator;

enum ESceneNodeRenderPass
{
	ESNRP_NONE = 0,
	ESNRP_CAMERA,
	ESNRP_LIGHT,
	ESNRP_SKY_BOX,
	ESNRP_SOLID,
	ESNRP_TRANSPARENT,
	ESNRP_AUTOMATIC,
	ESNRP_TRANSPARENT_EFFECT,
	ESNRP_SHADOW,
	ESNRP_COUNT,
};

class SceneManager : public input::EventReceiver, public ReferenceCounted
{
public:
	// Destruktor
	virtual ~SceneManager()
	{
	}

	// Fügt eine neue Kamera zum Scene-Graph hinzu
	virtual StrongRef<CameraSceneNode> AddCameraSceneNode(const math::vector3f& position = math::vector3f::ZERO,
		const math::vector3f& direction = math::vector3f::UNIT_Z, bool makeActive = true) = 0;

	// Fügt ein neues Modell zum Scene-Graph hinzu
	virtual StrongRef<MeshSceneNode> AddMeshSceneNode(Mesh* mesh,
		bool addIfMeshIsEmpty = true,
		const math::vector3f& position = math::vector3f::ZERO,
		const math::quaternionf& orient = math::quaternionf(0.0f, 0.0f, 0.0f, 1.0f),
		float scale = 1.0f) = 0;

	// Fügt eine Sky-box zum Scene-Graph hinzu
	virtual StrongRef<SceneNode> AddSkyBoxSceneNode(video::CubeTexture* skyTexture) = 0;

	virtual StrongRef<LightSceneNode> AddLightSceneNode(const math::vector3f position = math::vector3f::ZERO,
		const video::Colorf color = video::Colorf(1.0f, 1.0f, 1.0f),
		float range = 100.0f) = 0;

	virtual StrongRef<SceneNode> AddEmptySceneNode(SceneNode* parent = nullptr) = 0;

	virtual StrongRef<SceneNode> AddSceneNode(core::Name type, SceneNode* parent = nullptr) = 0;


	//--------------------------------------------------------------------------
	// Componenten

	// Erstellt einen Rotationsanimator
	// vRot = Rotationsgeschwindigkeit in Rad/Sekunde
	virtual StrongRef<SceneNodeComponent> AddRotationAnimator(SceneNode* addTo, const math::vector3f& axis=math::vector3f::UNIT_Y, math::anglef rotSpeed = math::anglef::Degree(45.0f)) = 0;

	virtual StrongRef<SceneNodeComponent> AddLinearMoveAnimator(SceneNode* addTo,
		const math::line3df& line,
		float duration,
		bool jumpBack = false,
		u32 count = std::numeric_limits<u32>::max()) = 0;

	// Erstellt eine Component, mit dem sich die Kamera kontrollieren lässt
	virtual StrongRef<CameraFPSAnimator> AddCameraFPSAnimator(CameraSceneNode* addTo, float moveSpeed=4.0f, math::anglef rotSpeed = math::anglef::Degree(90.0f),
		bool noVerticalMovement = false,
		math::anglef maxVerticalAngle = math::anglef::Degree(89.0f)) = 0;

	//--------------------------------------------------------------------------

	// Liefert die Video-Schnittstelle
	virtual video::VideoDriver* GetDriver() = 0;

	// Liefert das Datei-System
	virtual io::FileSystem* GetFileSystem() = 0;

	virtual video::MaterialLibrary* GetMaterialLibrary() = 0;

	virtual video::ImageSystem* GetImageSystem() const = 0;

	virtual core::ReferableFactory* GetReferableFactory() const = 0;
	virtual core::ResourceSystem* GetResourceSystem() const = 0;

	// Liefert den Modell-Cache
	virtual MeshCache* GetMeshCache() = 0;

	// Liefert den Wurzelknoten
	virtual SceneNode* GetRootSceneNode() = 0;

	// Fragt einen Scene-node anhand seines Typs ab
	virtual SceneNode* GetSceneNodeByType(core::Name type, SceneNode* start = nullptr, u32 tags=0) = 0;

	// Fragt mehrere Scene-Nodes anhand ihres Typs ab
	virtual bool GetSceneNodeArrayByType(core::Name type, core::array<SceneNode*>& array, SceneNode* start = nullptr, u32 tags=0) = 0;

	// Fragt die aktive Kamera ab
	virtual StrongRef<CameraSceneNode> GetActiveCamera() = 0;

	// Setzt die aktive Kamera
	virtual void SetActiveCamera(CameraSceneNode* camera) = 0;

	// Zeichnet alles
	virtual void DrawAll() = 0;

	// Animiert alles
	virtual void AnimateAll(float secsPassed) = 0;

	// Registriet einen Scenenode zum Zeichnen
	virtual bool RegisterNodeForRendering(SceneNode* node, ESceneNodeRenderPass renderPass = ESNRP_AUTOMATIC) = 0;

	// Fügt den Knoten zur DeletionQueue hinzu
	virtual void AddToDeletionQueue(SceneNode* node) = 0;

	// Leert die Deletionqueue
	virtual void ClearDeletionQueue() = 0;

	// Löscht alles
	virtual void Clear() = 0;

	// Setzt die Hintergrund-Beleuchtung
	virtual void SetAmbient(video::Color ambient) = 0;

	// Fragt die Hintergrund-Beleuchtung ab
	virtual video::Color GetAmbient() = 0;

	// Der aktuelle Renderingpass
	virtual ESceneNodeRenderPass GetActRenderPass() const = 0;

	virtual void RegisterEventReceiver(SceneNode* receiver) = 0;
	virtual void UnregisterEventReceiver(SceneNode* receiver) = 0;

	virtual void AddPipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over) = 0;
	virtual void RemovePipelineOverwrite(ESceneNodeRenderPass pass, const video::PipelineOverwrite& over) = 0;
};

}    // namespace scene
}    // namespace lux

#endif