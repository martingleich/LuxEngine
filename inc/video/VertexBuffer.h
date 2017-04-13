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
	VertexBuffer() : HardwareBuffer(EHardwareBufferType::Vertex) { }

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

}    //namespace scene
}    //namespace lux

#endif