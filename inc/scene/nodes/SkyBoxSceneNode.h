#ifndef INCLUDED_SKYBOXSCENENODE_H
#define INCLUDED_SKYBOXSCENENODE_H
#include "scene/SceneNode.h"
#include "video/VertexTypes.h"
#include "video/CubeTexture.h"

namespace lux
{
namespace scene
{
class SkyBoxSceneNode : public SceneNode
{
public:
	LUX_API void SetSkyTexture(video::CubeTexture* skyTexture);

	bool SetSceneManager(SceneManager* mgr);
	void OnRegisterSceneNode();
	void Render();
	video::Material* GetMaterial(size_t id);
	void SetMaterial(size_t i, video::Material* m);
	size_t GetMaterialCount() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	StrongRef<video::CubeTexture> m_SkyTexture;
	StrongRef<video::Material> m_Material;
};

} // namespace scene
} // namespace lux

#endif