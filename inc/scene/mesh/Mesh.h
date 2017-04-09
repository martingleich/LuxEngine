#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H
#include "math/aabbox3d.h"
#include "resources/ResourceSystem.h"

namespace lux
{
namespace video
{
class SubMesh;
class Material;
}
namespace scene
{

//! A complex mesh composed of simple Sub-Meshs
class Mesh : public core::Resource
{
public:
	virtual ~Mesh()
	{
	}

	//! Removes all Submeshes
	virtual void Clear() = 0;

	//! Return the number of Submeshs
	virtual size_t GetSubMeshCount() const = 0;

	//! Retrieve a sub mesh by index
	/**
	\param i The index of the sub mesh to return
	\return The submesh
	*/
	virtual const video::SubMesh* GetSubMesh(size_t i) const = 0;

	//! Retrieve a sub mesh by index
	/**
	\param i The index of the sub mesh to return
	\return The submesh
	*/
	virtual StrongRef<video::SubMesh> GetSubMesh(size_t i) = 0;

	//! Returns the Bounding-box of the whole mesh
	virtual const math::aabbox3df& GetBoundingBox() const = 0;

	//! Add a new Submesh to the mesh
	/**
	The bounding box isn´t recalculated automaticly
	\param subMesh The submesh to add
	*/
	virtual void AddSubMesh(video::SubMesh* subMesh) = 0;

	//! Removes a Submesh by index
	virtual void RemoveSubMesh(size_t index) = 0;

	//! Removes a submesh by pointer
	virtual void RemoveSubMesh(video::SubMesh* subMesh) = 0;

	//! Recalculates the bounding box
	/**
	If a user-defined bounding box is set, it will be deleted
	*/
	virtual void RecalculateBoundingBox() = 0;

	//! Set a user-defined bounding box
	virtual void SetBoundingBox(const math::aabbox3df& box) = 0;

	//! Get a submesh material by index.
	virtual video::Material& GetMaterial(size_t index) = 0;

	//! Get a submesh material by index.
	virtual const video::Material& GetMaterial(size_t index) const = 0;

	virtual core::Name GetResourceType() const
	{
		return core::ResourceType::Mesh;
	}
};

}    

}    


#endif