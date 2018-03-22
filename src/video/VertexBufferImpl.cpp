#include "video/VertexBufferImpl.h"

namespace lux
{
namespace video
{

VertexBufferImpl::VertexBufferImpl(BufferManager* mgr) :
	m_Stream(0),
	m_Format(VertexFormat::STANDARD),
	m_Manager(mgr)
{
	m_Stride = m_Format.GetStride(m_Stream);

	m_Manager->AddBuffer(this);
}

VertexBufferImpl::~VertexBufferImpl()
{
	m_Manager->RemoveBuffer(this);
}

void VertexBufferImpl::SetFormat(const VertexFormat& format, int stream, const void* init)
{
	int stride = format.GetStride(stream);
	if(!format.IsValid() || stride == 0)
		throw core::InvalidArgumentException("format", "Format is invalid");

	if(m_Data) {
		if(init) {
			for(int i = 0; i < m_Size; ++i)
				memcpy(m_Data + i*stride, init, stride);
		}
	}

	m_Format = format;
	m_Stream = stream;
	m_Stride = stride;

	m_ChangeId++;
}

const VertexFormat& VertexBufferImpl::GetFormat() const
{
	return m_Format;
}

int VertexBufferImpl::GetStream() const
{
	return m_Stream;
}

int VertexBufferImpl::AddVertex(const void* vertex)
{
	return AddVertices(vertex, 1);
}

int VertexBufferImpl::AddVertices(const void* vertices, int count)
{
	if(m_Cursor + count - 1 >= m_Size) {
		Reserve(m_Cursor + count);
		m_Size += count;
	}

	SetVertices(vertices, count, m_Cursor);

	int ret = m_Cursor;
	m_Cursor += count;
	return ret;
}

void VertexBufferImpl::SetVertex(const void* vertex, int n)
{
	SetVertices(vertex, 1, n);
}
void VertexBufferImpl::SetVertices(const void* vertices, int count, int n)
{
	memcpy(Pointer(n, count), vertices, count*m_Stride);
}
void VertexBufferImpl::GetVertex(void* ptr, int n) const
{
	GetVertices(ptr, 1, n);
}
void VertexBufferImpl::GetVertices(void* ptr, int count, int n) const
{
	memcpy(ptr, Pointer(n, count), count*m_Stride);
}

}

}

