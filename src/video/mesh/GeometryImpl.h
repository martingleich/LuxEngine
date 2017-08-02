#ifndef INCLUDED_SUBMESH_IMPL_H
#define INCLUDED_SUBMESH_IMPL_H
#include "video/mesh/Geometry.h"
#include "video/Material.h"

namespace lux
{
namespace video
{

class GeometryImpl : public Geometry
{
public:
	GeometryImpl();
	GeometryImpl(VertexBuffer* vertices, IndexBuffer* indices);
	void SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType = video::EPrimitiveType::Triangles);
	video::EPrimitiveType GetPrimitiveType() const;
	u32 GetPrimitiveCount() const;
	void SetPrimitiveType(video::EPrimitiveType type);
	void SetVertices(VertexBuffer* vertices);
	StrongRef<VertexBuffer> GetVertices();
	const VertexBuffer* GetVertices() const;
	u32 GetVertexCount() const;
	const video::VertexFormat& GetVertexFormat() const;
	void SetIndices(IndexBuffer* indices);
	const IndexBuffer* GetIndices() const;
	StrongRef<IndexBuffer> GetIndices();
	u32 GetIndexCount() const;
	video::EIndexFormat GetIndexType() const;
	const math::AABBoxF& GetBoundingBox() const;
	void SetBoundingBox(const math::AABBoxF& box);
	void RecalculateBoundingBox();
	u32 GetChangeId() const;
	
private:
	StrongRef<VertexBuffer> m_Vertices;
	StrongRef<IndexBuffer> m_Indices;

	EPrimitiveType m_PrimitveType;

	math::AABBoxF m_BoundingBox;
};

} // namespace video
} // namespace lux

#endif