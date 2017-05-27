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
	//! Get the currently used model
	/**
	\return The current model
	\ref SetMesh
	*/
	virtual StrongRef<Mesh> GetMesh() = 0;

	//! Set a new model
	/**
	\param mesh The mesh which is rendered by this scenenode
	\ref GetMesh
	*/
	virtual void SetMesh(Mesh* mesh) = 0;

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
	virtual void SetReadMaterialsOnly(bool state) = 0;

	//! The state of ReadMaterialsOnly
	/**
	\return The state of ReadMaterialsOnly
	\ref SetReadMaterialsOnly
	*/
	virtual bool GetReadMaterialsOnly() const = 0;
};

} // namespace scene
} // namespace lux

#endif