#ifndef INCLUDED_LUX_SCENE_SKYBOX_H
#define INCLUDED_LUX_SCENE_SKYBOX_H
#include "scene/Component.h"
#include "scene/Renderable.h"

namespace lux
{
namespace video
{
class BaseTexture;
class CubeTexture;
class Material;
}
namespace scene
{

class SkyBox : public Component, public Renderable
{
	LX_REFERABLE_MEMBERS_API(SkyBox, LUX_API);
public:
	LUX_API SkyBox();
	LUX_API SkyBox(const SkyBox& other);
	LUX_API ~SkyBox();

	LUX_API virtual void VisitRenderables(RenderableVisitor* visitor, ERenderableTags tags);
	LUX_API virtual void Render(Node* node, video::Renderer* renderer, const SceneData& data);
	LUX_API virtual ERenderPass GetRenderPass() const;

	LUX_API virtual void UseCubeTexture(bool cube);
	LUX_API virtual bool IsUsingCubeTexture() const;

	LUX_API virtual void SetSkyTexture(video::BaseTexture* skyTexture);
	LUX_API virtual StrongRef<video::BaseTexture> GetSkyTexture() const;

	LUX_API virtual video::Material* GetMaterial();
	LUX_API virtual const video::Material* GetMaterial() const;
	LUX_API virtual void SetMaterial(video::Material* m);

	LUX_API virtual const math::AABBoxF& GetBoundingBox() const;

protected:
	StrongRef<video::BaseTexture> m_SkyTexture;
	StrongRef<video::Material> m_Material;
	bool m_UseCubeTexture;
};

} // namespace scene
} // namespace lux

#endif