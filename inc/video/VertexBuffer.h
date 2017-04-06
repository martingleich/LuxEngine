#ifndef INCLUDED_VERTEXBUFFER_H
#define INCLUDED_VERTEXBUFFER_H
#include "video/HardwareBuffer.h"
#include "video/VertexFormats.h"

namespace lux
{

namespace video
{

class VertexBuffer : public HardwareBuffer
{
public:
	VertexBuffer(BufferManager* mgr) : HardwareBuffer(mgr, EHardwareBufferType::Vertex)
	{
	}
	virtual const VertexFormat& GetFormat() const = 0;
	virtual void SetFormat(const VertexFormat& format, void* init = nullptr) = 0;
	virtual u32 GetStream() const = 0;
	virtual void SetFormat(const VertexFormat& format, u32 stream, void* init = nullptr) = 0;
	virtual u32 AddVertex(void* vertex) = 0;
	virtual u32 AddVertices(void* vertices, u32 count) = 0;
	virtual void SetVertex(void* vertex, u32 n) = 0;
	virtual void SetVertices(void* vertices, u32 count, u32 n) = 0;
	virtual void GetVertex(void* ptr, u32 n) const = 0;
	virtual void GetVertices(void* ptr, u32 count, u32 n) const = 0;
};

class VertexBufferImpl : public VertexBuffer
{
	friend class BufferManagerNull;
private:
	u32 m_Stream;
	VertexFormat m_Format;

private:
	VertexBufferImpl(BufferManager* mgr);

public:
	const VertexFormat& GetFormat() const;
	void SetFormat(const VertexFormat& format, void* init = nullptr);
	u32 GetStream() const;
	void SetFormat(const VertexFormat& format, u32 stream, void* init = nullptr);
	u32 AddVertex(void* vertex);
	u32 AddVertices(void* vertices, u32 count);
	void SetVertex(void* vertex, u32 n);
	void SetVertices(void* vertices, u32 count, u32 n);
	void GetVertex(void* ptr, u32 n) const;
	void GetVertices(void* ptr, u32 count, u32 n) const;
};

}    //namespace scene
}    //namespace lux

#endif