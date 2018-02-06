#ifndef INCLUDED_SUBMESH_H
#define INCLUDED_SUBMESH_H
#include "core/ReferenceCounted.h"
#include "math/AABBox.h"
#include "video/VideoEnums.h"

namespace lux
{
namespace video
{
class VertexBuffer;
class IndexBuffer;
class VertexFormat;

//! The most simple mesh, contains a index- and vertexbuffer
class Geometry : public ReferenceCounted
{
public:
	LUX_API Geometry();

	LUX_API virtual ~Geometry();

	//! Set a new vertex and index buffer
	/**
	\param vertices The new vertexbuffer, must not be null
	\param indices The new indexbuffer, can be null
	\param primitveType The primitve type in which way the data is interpreted
	*/
	LUX_API virtual void SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType = EPrimitiveType::Triangles);

	//! Get the primitive type used by this mesh
	/**
	\return primitive type
	*/
	LUX_API virtual EPrimitiveType GetPrimitiveType() const;

	//! Set a new primitive type
	/**
	\param type The new primitive type
	*/
	LUX_API virtual void SetPrimitiveType(EPrimitiveType type);

	//! Get the total number of primitves in this mesh
	/**
	Degenerated primitves are also counted.
	\return The number of primitives
	*/
	LUX_API virtual u32 GetPrimitiveCount() const;

	//! Set new vertices for the submesh
	/**
	\param vertices The vertices must not be zero
	*/
	LUX_API virtual void SetVertices(VertexBuffer* vertices);

	//! Get the vertices used by the submesh
	/**
	\return The vertices
	*/
	LUX_API virtual const VertexBuffer* GetVertices() const;

	//! Get the vertices used by the submesh
	/**
	\return The vertices
	*/
	LUX_API virtual StrongRef<VertexBuffer> GetVertices();

	//! The total number of vertices in the submesh
	LUX_API virtual u32 GetVertexCount() const;

	//! The vertextype used by the submesh
	LUX_API virtual const VertexFormat& GetVertexFormat() const;

	//! Set a new indexbuffer
	/**
	\param indices The new indices, can be null
	*/
	LUX_API virtual void SetIndices(IndexBuffer* indices) ;
	//! Get the indices used by the submesh
	/**
	\return The indices
	*/
	LUX_API virtual const IndexBuffer* GetIndices() const;

	//! Get the indices used by the submesh
	/**
	\return The indices
	*/
	LUX_API virtual StrongRef<IndexBuffer> GetIndices();

	//! The total number of indices in the submesh
	LUX_API virtual u32 GetIndexCount() const;

	//! The indexformat used by the submesh
	LUX_API virtual video::EIndexFormat GetIndexFormat() const;

	//! The bounding box of the submesh data
	LUX_API virtual const math::AABBoxF& GetBoundingBox() const;

	//! Set a user defined bounding box for the data
	/**
	\param box The user's bounding box
	*/
	LUX_API virtual void SetBoundingBox(const math::AABBoxF& box);

	//! Recalculate the bounding box from the vertex data
	/**
	Should be called after changing the vertexbuffer.
	And you didn't set a bounding box via SetBoundingBox
	*/
	LUX_API virtual void RecalculateBoundingBox();

	//! Are the triangles drawn in counter-clockwise or clockwise order.
	LUX_API virtual void SetWindingOrder(bool counterClockwise);

	//! Are the triangles drawn in counter-clockwise or clockwise order.
	LUX_API virtual bool GetWindingOrder() const;

	//! Get the change id of the geometry
	LUX_API virtual u32 GetChangeId() const;

protected:
	StrongRef<VertexBuffer> m_Vertices;
	StrongRef<IndexBuffer> m_Indices;

	EPrimitiveType m_PrimitiveType;

	math::AABBoxF m_BoundingBox;
	bool m_WindingOrder;
};

} // namespace video
} // namespace lux

#endif