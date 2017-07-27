#include "video/mesh/GeometryImpl.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexFormats.h"

namespace lux
{
namespace video
{

GeometryImpl::GeometryImpl() :
	m_PrimitveType(EPrimitiveType::Triangles)
{
}

GeometryImpl::GeometryImpl(VertexBuffer* vertices, IndexBuffer* indices) :
	m_Vertices(vertices),
	m_Indices(indices),
	m_PrimitveType(EPrimitiveType::Triangles)
{
}

void GeometryImpl::SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType)
{
	m_Vertices = vertices;
	m_Indices = indices;
	m_PrimitveType = primitiveType;
}

video::EPrimitiveType GeometryImpl::GetPrimitiveType() const
{
	return m_PrimitveType;
}

u32 GeometryImpl::GetPrimitiveCount() const
{
	u32 pointCount;
	if(m_Indices != nullptr)
		pointCount = m_Indices->GetSize();
	else
		pointCount = m_Vertices->GetSize();

	switch(m_PrimitveType) {
	case video::EPrimitiveType::Lines:
		return pointCount / 2;
	case video::EPrimitiveType::LineStrip:
		return pointCount > 0 ? pointCount - 1 : 0;
	case video::EPrimitiveType::Points:
		return pointCount;
	case video::EPrimitiveType::Triangles:
		return pointCount / 3;
	case video::EPrimitiveType::TriangleFan:
	case video::EPrimitiveType::TriangleStrip:
		return pointCount > 2 ? pointCount - 2 : 0;
	};

	return 0;
}

void GeometryImpl::SetPrimitiveType(video::EPrimitiveType type)
{
	m_PrimitveType = type;
}

void GeometryImpl::SetVertices(VertexBuffer* vertices)
{
	m_Vertices = vertices;
}

StrongRef<VertexBuffer> GeometryImpl::GetVertices()
{
	return m_Vertices;
}

const VertexBuffer* GeometryImpl::GetVertices() const
{
	return m_Vertices;
}

u32 GeometryImpl::GetVertexCount() const
{
	if(!m_Vertices)
		return 0;
	else
		return m_Vertices->GetSize();
}

const video::VertexFormat& GeometryImpl::GetVertexFormat() const
{
	if(!m_Vertices)
		return video::VertexFormat::STANDARD;
	else
		return m_Vertices->GetFormat();
}

void GeometryImpl::SetIndices(IndexBuffer* indices)
{
	m_Indices = indices;
}

const IndexBuffer* GeometryImpl::GetIndices() const
{
	return m_Indices;
}

StrongRef<IndexBuffer> GeometryImpl::GetIndices()
{
	return m_Indices;
}

u32 GeometryImpl::GetIndexCount() const
{
	if(!m_Indices)
		return 0;
	else
		return m_Indices->GetSize();
}

video::EIndexFormat GeometryImpl::GetIndexType() const
{
	if(!m_Indices)
		return video::EIndexFormat::Bit16;
	else
		return m_Indices->GetType();
}
const math::AABBoxF& GeometryImpl::GetBoundingBox() const
{
	return m_BoundingBox;
}

void GeometryImpl::SetBoundingBox(const math::AABBoxF& box)
{
	m_BoundingBox = box;
}

void GeometryImpl::RecalculateBoundingBox()
{
	if(GetVertexCount() == 0) {
		m_BoundingBox.Set(0.0f, 0.0f, 0.0f);
	} else {
		const u32 posOffset = GetVertexFormat().GetElement(0, video::VertexElement::EUsage::Position).offset;

		const u8* vertex = reinterpret_cast<const u8*>(m_Vertices->Pointer_c(0, 1));
		m_BoundingBox.Set(*reinterpret_cast<const math::Vector3F*>(vertex + posOffset));

		for(u32 i = 1; i < m_Vertices->GetSize(); ++i) {
			vertex = reinterpret_cast<const u8*>(m_Vertices->Pointer_c(i, 1));
			m_BoundingBox.AddPoint(*reinterpret_cast<const math::Vector3F*>(vertex + posOffset));
		}
	}
}

}
}
