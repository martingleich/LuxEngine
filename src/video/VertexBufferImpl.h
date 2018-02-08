#ifndef INCLUDED_VERTEX_BUFFER_IMPL_H
#define INCLUDED_VERTEX_BUFFER_IMPL_H
#include "video/VertexBuffer.h"
#include "video/HardwareBufferManager.h"
#include "video/VertexFormat.h"

namespace lux
{
namespace video
{

class VertexBufferImpl : public VertexBuffer
{
public:
	VertexBufferImpl(BufferManager* mgr);
	~VertexBufferImpl();

	void SetFormat(const VertexFormat& format, u32 stream, const void* init = nullptr);

	const VertexFormat& GetFormat() const;
	u32 GetStream() const;

	u32 AddVertex(const void* vertex);
	u32 AddVertices(const void* vertices, u32 count);
	void SetVertex(const void* vertex, u32 n);
	void SetVertices(const void* vertices, u32 count, u32 n);
	void GetVertex(void* ptr, u32 n) const;
	void GetVertices(void* ptr, u32 count, u32 n) const;

	void SetHandle(void* handle) { m_Handle = handle; }
	void* GetHandle() const { return m_Handle; }
	BufferManager* GetManager() { return m_Manager; }
	void UpdateByManager(u32 group) { m_Manager->UpdateBuffer(this, group); }

private:
	u32 m_Stream;
	VertexFormat m_Format;

	BufferManager* m_Manager;
	void* m_Handle;
};

}
}

#endif // #ifndef INCLUDED_VERTEX_BUFFER_IMPL_H
