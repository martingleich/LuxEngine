#ifndef INCLUDED_SCENE_SKYBOX_H
#define INCLUDED_SCENE_SKYBOX_H
#include "scene/Component.h"
#include "scene/Renderable.h"

namespace lux
{
namespace video
{
class CubeTexture;
}
namespace scene
{

class SkyBox : public Component, public Renderable
{
public:
	LUX_API SkyBox();
	LUX_API SkyBox(const SkyBox& other);
	LUX_API ~SkyBox();

	LUX_API virtual void VisitRenderables(RenderableVisitor* visitor, bool noDebug);
	LUX_API virtual void Render(Node* node, video::Renderer* renderer, ERenderPass pass);
	LUX_API virtual ERenderPass GetRenderPass() const;

	LUX_API virtual void UseCubeTexture(bool cube);
	LUX_API virtual bool IsUsingCubeTexture() const;

	LUX_API virtual void SetSkyTexture(video::BaseTexture* skyTexture);
	LUX_API virtual StrongRef<video::BaseTexture> GetSkyTexture() const;

	LUX_API virtual video::Material* GetMaterial(size_t id);
	LUX_API virtual const video::Material* GetMaterial(size_t id) const;
	LUX_API virtual void SetMaterial(size_t i, video::Material* m);
	LUX_API virtual size_t GetMaterialCount() const;

	LUX_API virtual const math::aabbox3df& GetBoundingBox() const;

	LUX_API virtual core::Name GetReferableType() const;
	LUX_API virtual StrongRef<Referable> Clone() const;

protected:
	StrongRef<video::BaseTexture> m_SkyTexture;
	StrongRef<video::Material> m_Material;
	bool m_UseCubeTexture;
};

} // namespace scene
} // namespace lux

#endif