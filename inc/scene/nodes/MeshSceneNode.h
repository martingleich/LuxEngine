#ifndef INCLUDED_MESHSCENENODE_H
#define INCLUDED_MESHSCENENODE_H
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{
class Mesh;

//! Represent a single mesh in the scenegraph
class MeshSceneNode : public SceneNode
{
public:
	MeshSceneNode();

	//! Get the currently used model
	/**
	\return The current model
	\ref SetMesh
	*/
	LUX_API StrongRef<Mesh> GetMesh();

	//! Set a new model
	/**
	\param mesh The mesh which is rendered by this scenenode
	\ref GetMesh
	*/
	LUX_API void SetMesh(Mesh* mesh);

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
	LUX_API void SetReadMaterialsOnly(bool state);

	//! The state of ReadMaterialsOnly
	/**
	\return The state of ReadMaterialsOnly
	\ref SetReadMaterialsOnly
	*/
	LUX_API bool GetReadMaterialsOnly() const;

	void OnRegisterSceneNode();
	void Render();
	const math::aabbox3df& GetBoundingBox() const;

	video::Material* GetMaterial(size_t index);
	void SetMaterial(size_t index, video::Material* m);

	size_t GetMaterialCount() const;

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	void CopyMaterials();

private:
	StrongRef<Mesh> m_Mesh;
	bool m_OnlyReadMaterials;
	core::array<StrongRef<video::Material>> m_Materials;
	math::aabbox3df m_BoundingBox;
	RenderTransform m_RenderTransform;
};

} // namespace scene
} // namespace lux

#endif