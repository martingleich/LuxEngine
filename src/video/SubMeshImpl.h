#ifndef INCLUDED_SUBMESH_IMPL_H
#define INCLUDED_SUBMESH_IMPL_H
#include "video/SubMesh.h"
#include "video/Material.h"

namespace lux
{
namespace video
{

class SubMeshImpl : public SubMesh
{
public:
	SubMeshImpl();
	SubMeshImpl(VertexBuffer* vertices, IndexBuffer* indices);
	void SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType = video::EPT_TRIANGLES);
	video::EPrimitiveType GetPrimitiveType() const;
	u32 GetPrimitveCount() const;
	void SetPrimitiveType(video::EPrimitiveType type);
	const video::Material& GetMaterial() const;
	video::Material& GetMaterial();
	void SetMaterial(const video::Material& material);
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
	const math::aabbox3df& GetBoundingBox() const;
	void SetBoundingBox(const math::aabbox3df& box);
	void RecalculateBoundingBox();

private:
	Material m_Material;
	StrongRef<VertexBuffer> m_Vertices;
	StrongRef<IndexBuffer> m_Indices;
	EPrimitiveType m_PrimitveType;
	math::aabbox3df m_BoundingBox;
};

}    

}    


#endif