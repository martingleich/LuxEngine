#ifndef INCLUDED_CSKYBOXSCENENODE_H
#define INCLUDED_CSKYBOXSCENENODE_H

#include "scene/SceneNode.h"
#include "video/VertexTypes.h"
#include "video/CubeTexture.h"

namespace lux
{
namespace scene
{
class SkyBoxSceneNodeImpl : public SceneNode
{
public:
	SkyBoxSceneNodeImpl() = default;
	SkyBoxSceneNodeImpl(const SkyBoxSceneNodeImpl& other) = default;

	bool SetSceneManager(SceneManager* mgr);
	void OnRegisterSceneNode();
	void Render();
	void SetSkyTexture(video::CubeTexture* skyTexture);
	video::Material& GetMaterial(size_t id);
	size_t GetMaterialCount() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	StrongRef<video::CubeTexture> m_SkyTexture;
	video::Material m_Material;
};

}    

}    


#endif