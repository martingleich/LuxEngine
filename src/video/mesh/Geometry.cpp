#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexFormats.h"

namespace lux
{
namespace video
{

Geometry::Geometry() :
	m_PrimitiveType(EPrimitiveType::Triangles),
	m_WindingOrder(true)
{
}

Geometry::~Geometry()
{
}

void Geometry::SetBuffer(VertexBuffer* vertices, IndexBuffer* indices, EPrimitiveType primitiveType)
{
	m_Vertices = vertices;
	m_Indices = indices;
	m_PrimitiveType = primitiveType;
}

video::EPrimitiveType Geometry::GetPrimitiveType() const
{
	return m_PrimitiveType;
}

u32 Geometry::GetPrimitiveCount() const
{
	u32 pointCount;
	if(m_Indices != nullptr)
		pointCount = m_Indices->GetSize();
	else
		pointCount = m_Vertices->GetSize();

	return video::GetPrimitiveCount(m_PrimitiveType, pointCount);
}

void Geometry::SetPrimitiveType(video::EPrimitiveType type)
{
	m_PrimitiveType = type;
}

void Geometry::SetVertices(VertexBuffer* vertices)
{
	m_Vertices = vertices;
}

StrongRef<VertexBuffer> Geometry::GetVertices()
{
	return m_Vertices;
}

const VertexBuffer* Geometry::GetVertices() const
{
	return m_Vertices;
}

u32 Geometry::GetVertexCount() const
{
	if(!m_Vertices)
		return 0;
	else
		return m_Vertices->GetSize();
}

const video::VertexFormat& Geometry::GetVertexFormat() const
{
	if(!m_Vertices)
		return video::VertexFormat::STANDARD;
	else
		return m_Vertices->GetFormat();
}

void Geometry::SetIndices(IndexBuffer* indices)
{
	m_Indices = indices;
}

const IndexBuffer* Geometry::GetIndices() const
{
	return m_Indices;
}

StrongRef<IndexBuffer> Geometry::GetIndices()
{
	return m_Indices;
}

u32 Geometry::GetIndexCount() const
{
	if(!m_Indices)
		return 0;
	else
		return m_Indices->GetSize();
}

video::EIndexFormat Geometry::GetIndexFormat() const
{
	if(!m_Indices)
		return video::EIndexFormat::Bit16;
	else
		return m_Indices->GetFormat();
}
const math::AABBoxF& Geometry::GetBoundingBox() const
{
	return m_BoundingBox;
}

void Geometry::SetBoundingBox(const math::AABBoxF& box)
{
	m_BoundingBox = box;
}

void Geometry::RecalculateBoundingBox()
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

void Geometry::SetWindingOrder(bool counterClockwise)
{
	m_WindingOrder = counterClockwise;
}

bool Geometry::GetWindingOrder() const
{
	return m_WindingOrder;
}

u32 Geometry::GetChangeId() const
{
	return m_Vertices ? m_Vertices->GetChangeId() : 0 + m_Indices ? m_Indices->GetChangeId() : 0;
}
}
}
