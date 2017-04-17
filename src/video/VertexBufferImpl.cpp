#include "video/VertexBufferImpl.h"

namespace lux
{
namespace video
{

VertexBufferImpl::VertexBufferImpl(BufferManager* mgr) :
	m_Manager(mgr),
	m_Format(VertexFormat::STANDARD),
	m_Stream(0)
{
	m_Stride = m_Format.GetStride(m_Stream);

	m_Manager->AddBuffer(this);
}

VertexBufferImpl::~VertexBufferImpl()
{
	m_Manager->RemoveBuffer(this);
}

const VertexFormat& VertexBufferImpl::GetFormat() const
{
	return m_Format;
}

void VertexBufferImpl::SetFormat(const VertexFormat& format, void* init)
{
	SetFormat(format, 0, init);
}

u32 VertexBufferImpl::GetStream() const
{
	return m_Stream;
}

void VertexBufferImpl::SetFormat(const VertexFormat& format, u32 stream, void* init)
{
	u32 stride = format.GetStride(stream);

	if(m_Data) {
		if(init)
			for(u32 i = 0; i < m_Size; ++i)
				memcpy(m_Data + i*stride, init, stride);
	}

	m_Format = format;
	m_Stream = stream;
	m_Stride = stride;
}

u32 VertexBufferImpl::AddVertex(void* vertex)
{
	return AddVertices(vertex, 1);
}

u32 VertexBufferImpl::AddVertices(void* vertices, u32 count)
{
	if(m_Cursor + count - 1>= m_Size) {
		Reserve(m_Cursor + count);
		m_Size += count;
	}

	SetVertices(vertices, count, m_Cursor);

	u32 ret = m_Cursor;
	m_Cursor += count;
	return ret;
}

void VertexBufferImpl::SetVertex(void* vertex, u32 n)
{
	SetVertices(vertex, 1, n);
}
void VertexBufferImpl::SetVertices(void* vertices, u32 count, u32 n)
{
	memcpy(Pointer(n, count), vertices, count*m_Stride);
}
void VertexBufferImpl::GetVertex(void* ptr, u32 n) const
{
	GetVertices(ptr, 1, n);
}
void VertexBufferImpl::GetVertices(void* ptr, u32 count, u32 n) const
{
	memcpy(ptr, Pointer(n, count), count*m_Stride);
}

}    

}    
