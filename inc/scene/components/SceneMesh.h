#ifndef INCLUDED_SCENE_MESH_H
#define INCLUDED_SCENE_MESH_H
#include "scene/Component.h"
#include "math/aabbox3d.h"
#include "video/Material.h"
#include "scene/Renderable.h"

namespace lux
{
namespace video
{
class Mesh;
}
namespace scene
{

//! Represent a single mesh in the scenegraph
class Mesh : public Component, public Renderable
{
public:
	LUX_API Mesh();
	LUX_API Mesh(const Mesh& other);
	LUX_API ~Mesh();

	//! Get the currently used model
	/**
	\return The current model
	\ref SetMesh
	*/
	LUX_API virtual StrongRef<video::Mesh> GetMesh();

	//! Set a new model
	/**
	\param mesh The mesh which is rendered by this scenenode
	\ref GetMesh
	*/
	LUX_API virtual void SetMesh(video::Mesh* mesh);

	// Benutzt der Scene-Node nur die Materialien der Sub-Meshes
	// d.h. Wenn man das material der Sub-Meshes ändert, ändert es sich in allen
	// Scene-Nodes mit.
	//! Should materials be copied
	/**
	If ReadMaterialsOnly is set, the scene node which access the mesh material directly.
	If you change the mesh material all nodes will show this new material.
	If ReadMaterialsOnly is not set, a copy of all materials will be created. And all changes
	to the node will only affect these.\n
	The default state is set.
	\param state Should ReadMaterialOnly be set
	\ref GetReadMaterialsOnly
	*/
	LUX_API virtual void SetReadMaterialsOnly(bool state);

	//! The state of ReadMaterialsOnly
	/**
	\return The state of ReadMaterialsOnly
	\ref SetReadMaterialsOnly
	*/
	LUX_API virtual bool GetReadMaterialsOnly() const;

	LUX_API virtual void VisitRenderables(RenderableVisitor* visitor, bool noDebug);
	LUX_API virtual void Render(const Node* node, video::Renderer* renderer, ERenderPass pass);
	LUX_API virtual ERenderPass GetRenderPass() const;

	LUX_API virtual video::Material* GetMaterial(size_t index);
	LUX_API virtual const video::Material* Mesh::GetMaterial(size_t index) const;
	LUX_API virtual void SetMaterial(size_t index, video::Material* m);

	LUX_API virtual size_t GetMaterialCount() const;

	LUX_API virtual const math::aabbox3df& GetBoundingBox() const;

	LUX_API virtual core::Name GetReferableSubType() const;
	LUX_API virtual StrongRef<Referable> Clone() const;

private:
	void CopyMaterials();

protected:
	StrongRef<video::Mesh> m_Mesh;

	mutable bool m_DirtyMaterials;

	bool m_OnlyReadMaterials;
	core::array<StrongRef<video::Material>> m_Materials;

	math::aabbox3df m_BoundingBox;

	mutable ERenderPass m_RenderPass;
};

} // namespace scene
} // namespace lux

#endif