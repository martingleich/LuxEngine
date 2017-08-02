#ifndef INCLUDED_SUBMESH_H
#define INCLUDED_SUBMESH_H
#include "core/ReferenceCounted.h"
#include "math/AABBox.h"
#include "video/VideoEnums.h"

namespace lux
{
namespace video
{
class Material;
class VertexBuffer;
class IndexBuffer;
class VertexFormat;

//! The most simple mesh, contains a index- and vertexbuffer and a single material
class Geometry : public ReferenceCounted
{
public:
	virtual ~Geometry()
	{
	}

	//! Set a new vertex and index buffer
	/**
	\param vertices The new vertexbuffer, must not be null
	\param indices The new indexbuffer, can be null
	\param primitveType The primitve type in which way the data is interpreted
	*/
	virtual void SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType = EPrimitiveType::Triangles) = 0;

	//! Get the primitive type used by this mesh
	/**
	\return primitive type
	*/
	virtual EPrimitiveType GetPrimitiveType() const = 0;

	//! Set a new primitive type
	/**
	\param type The new primitive type
	*/
	virtual void SetPrimitiveType(EPrimitiveType type) = 0;

	//! Get the total number of primitves in this mesh
	/**
	Degenerated primitves are also counted.
	\return The number of primitives
	*/
	virtual u32 GetPrimitiveCount() const = 0;

	//! Set new vertices for the submesh
	/**
	\param vertices The vertices must not be zero
	*/
	virtual void SetVertices(VertexBuffer* vertices) = 0;

	//! Get the vertices used by the submesh
	/**
	\return The vertices
	*/
	virtual const VertexBuffer* GetVertices() const = 0;

	//! Get the vertices used by the submesh
	/**
	\return The vertices
	*/
	virtual StrongRef<VertexBuffer> GetVertices() = 0;

	//! The total number of vertices in the submesh
	virtual u32 GetVertexCount() const = 0;

	//! The vertextype used by the submesh
	virtual const VertexFormat& GetVertexFormat() const = 0;

	//! Set a new indexbuffer
	/**
	\param indices The new indices, can be null
	*/
	virtual void SetIndices(IndexBuffer* indices) = 0;

	//! Get the indices used by the submesh
	/**
	\return The indices
	*/
	virtual const IndexBuffer* GetIndices() const = 0;

	//! Get the indices used by the submesh
	/**
	\return The indices
	*/
	virtual StrongRef<IndexBuffer> GetIndices() = 0;

	//! The total number of indices in the submesh
	virtual u32 GetIndexCount() const = 0;

	//! The indexformat used by the submesh
	virtual video::EIndexFormat GetIndexType() const = 0;

	//! The bounding box of the submesh data
	virtual const math::AABBoxF& GetBoundingBox() const = 0;

	//! Set a user defined bounding box for the data
	/**
	\param box The user's bounding box
	*/
	virtual void SetBoundingBox(const math::AABBoxF& box) = 0;

	//! Recalculate the bounding box from the vertex data
	/**
	Should be called after changing the vertexbuffer.
	And you don't set a bounding box via SetBoundingBoxS
	*/
	virtual void RecalculateBoundingBox() = 0;

	virtual u32 GetChangeId() const = 0;
};

} // namespace video
} // namespace lux

#endif