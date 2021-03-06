#ifndef INCLUDED_LUX_SCENE_MESH_H
#define INCLUDED_LUX_SCENE_MESH_H
#include "scene/Component.h"
#include "math/AABBox.h"
#include "video/Material.h"

namespace lux
{
namespace video
{
class Mesh;
}
namespace scene
{

//! Represent a single mesh in the scenegraph
class Mesh : public Component
{
	LX_REFERABLE_MEMBERS_API(Mesh, LUX_API);

private:
	LUX_API Mesh();
	LUX_API Mesh(const Mesh& other);
public:
	LUX_API Mesh(video::Mesh* mesh);
	LUX_API ~Mesh();

	//! Get the currently used model.
	LUX_API StrongRef<video::Mesh> GetMesh();

	//! Set a new model
	LUX_API void SetMesh(video::Mesh* mesh);

	//! Should materials be copied
	/**
	If ReadMaterialsOnly is set, the scene node which access the mesh material directly.
	If you change the mesh material all nodes will show this new material.
	If ReadMaterialsOnly is not set, a copy of all materials will be created. And all material changes
	to the node will only affect ther original mesh.\n
	The default state is set.
	\param state Should ReadMaterialOnly be set
	\ref GetReadMaterialsOnly
	*/
	LUX_API void SetReadMaterialsOnly(bool state);

	//! The state of ReadMaterialsOnly
	/**
	\return The state of ReadMaterialsOnly
	\ref SetReadMaterialsOnly
	*/
	LUX_API bool GetReadMaterialsOnly() const;

	LUX_API void Render(const SceneRenderData& data) override;
	LUX_API RenderPassSet GetRenderPass() const override;

	LUX_API video::Material* GetMaterial(int index);
	LUX_API const video::Material* GetMaterial(int index) const;
	LUX_API void SetMaterial(int index, video::Material* m);

	LUX_API int GetMaterialCount() const;

	LUX_API const math::AABBoxF& GetBoundingBox() const override;

private:
	void CopyMaterials();

protected:
	StrongRef<video::Mesh> m_Mesh;

	bool m_OnlyReadMaterials;
	core::Array<StrongRef<video::Material>> m_Materials;

	math::AABBoxF m_BoundingBox;
};

} // namespace scene
} // namespace lux

#endif