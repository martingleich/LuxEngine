#ifndef INCLUDED_LUX_SCENE_SKYBOX_H
#define INCLUDED_LUX_SCENE_SKYBOX_H
#include "scene/Component.h"

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

class SkyBox : public Component
{
	LX_REFERABLE_MEMBERS_API(SkyBox, LUX_API);
public:
	LUX_API SkyBox();
	LUX_API SkyBox(const SkyBox& other);
	LUX_API ~SkyBox();

	LUX_API void Render(const SceneRenderData& data) override;
	LUX_API RenderPassSet GetRenderPass() const override;

	LUX_API void UseCubeTexture(bool cube);
	LUX_API bool IsUsingCubeTexture() const;

	LUX_API void SetSkyTexture(video::BaseTexture* skyTexture);
	LUX_API StrongRef<video::BaseTexture> GetSkyTexture() const;

	LUX_API video::Material* GetMaterial();
	LUX_API const video::Material* GetMaterial() const;
	LUX_API void SetMaterial(video::Material* m);

protected:
	StrongRef<video::BaseTexture> m_SkyTexture;
	StrongRef<video::Material> m_Material;
	bool m_UseCubeTexture;
};

} // namespace scene
} // namespace lux

#endif