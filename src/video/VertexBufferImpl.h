#ifndef INCLUDED_VERTEX_BUFFER_IMPL_H
#define INCLUDED_VERTEX_BUFFER_IMPL_H
#include "video/VertexBuffer.h"
#include "video/HardwareBufferManager.h"

namespace lux
{
namespace video
{
class VertexBufferImpl : public VertexBuffer
{
public:
	VertexBufferImpl(BufferManager* mgr);
	~VertexBufferImpl();

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
