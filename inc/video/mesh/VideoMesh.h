#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H
#include "math/aabbox3d.h"
#include "video/VertexFormats.h"
#include "video/HardwareBufferConstants.h"
#include "video/EPrimitiveType.h"
#include "resources/Resource.h"

namespace lux
{
namespace video
{
class SubMesh;
class Material;
class VideoDriver;

//! A complex mesh composed of simple Sub-Meshs
class Mesh : public core::Resource
{
public:
	virtual ~Mesh() {}

	//! Removes all Submeshes
	virtual void Clear() = 0;

	//! Return the number of Submeshs
	virtual size_t GetSubMeshCount() const = 0;

	//! Retrieve a sub mesh by index
	/**
	\param i The index of the sub mesh to return
	\return The submesh
	*/
	virtual const SubMesh* GetSubMesh(size_t i) const = 0;

	//! Retrieve a sub mesh by index
	/**
	\param i The index of the sub mesh to return
	\return The submesh
	*/
	virtual StrongRef<SubMesh> GetSubMesh(size_t i) = 0;

	//! Returns the Bounding-box of the whole mesh
	virtual const math::aabbox3df& GetBoundingBox() const = 0;

	//! Add a new Submesh to the mesh
	/**
	The bounding box isn´t recalculated automaticly
	\param subMesh The submesh to add
	*/
	virtual StrongRef<SubMesh> AddSubMesh(SubMesh* subMesh) = 0;

	//! Create a new submesh and add it to the mesh
	virtual StrongRef<SubMesh> AddSubMesh(const VertexFormat& vertexFormat, EHardwareBufferMapping vertexHWMapping, u32 vertexCount,
		EIndexFormat indexType, EHardwareBufferMapping indexHWMapping, u32 indexCount,
		EPrimitiveType primitiveType) = 0;

	//! Create a new submesh and add it to the mesh
	virtual StrongRef<SubMesh> AddSubMesh(const VertexFormat& vertexFormat = VertexFormat::STANDARD,
		EPrimitiveType primitiveType = EPrimitiveType::Triangles,
		u32 primitiveCount = 0,
		bool dynamic = false) = 0;

	//! Removes a Submesh by index
	virtual void RemoveSubMesh(size_t index) = 0;

	//! Removes a submesh by pointer
	virtual void RemoveSubMesh(SubMesh* subMesh) = 0;

	//! Recalculates the bounding box
	/**
	If a user-defined bounding box is set, it will be deleted
	*/
	virtual void RecalculateBoundingBox() = 0;

	//! Set a user-defined bounding box
	virtual void SetBoundingBox(const math::aabbox3df& box) = 0;

	//! Get a submesh material by index.
	virtual Material* GetMaterial(size_t index) = 0;

	//! Get a submesh material by index.
	virtual const Material* GetMaterial(size_t index) const = 0;

	virtual void SetMaterial(size_t index, Material* m) = 0;

	//! Get the video driver used by this mesh
	virtual VideoDriver* GetDriver() const = 0;

	virtual core::Name GetResourceType() const
	{
		return core::ResourceType::Mesh;
	}
};

} // namespace video
} // namespace lux

#endif