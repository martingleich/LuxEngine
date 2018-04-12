#ifndef INCLUDED_LUX_VERTEX_BUFFER_IMPL_H
#define INCLUDED_LUX_VERTEX_BUFFER_IMPL_H
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

	void SetFormat(const VertexFormat& format, int stream, bool moveOld, const void* init);

	const VertexFormat& GetFormat() const;
	int GetStream() const;

	int AddVertex(const void* vertex);
	int AddVertices(const void* vertices, int count);
	void SetVertex(const void* vertex, int n);
	void SetVertices(const void* vertices, int count, int n);
	void GetVertex(void* ptr, int n) const;
	void GetVertices(void* ptr, int count, int n) const;

	void SetHandle(void* handle) { m_Handle = handle; }
	void* GetHandle() const { return m_Handle; }
	BufferManager* GetManager() { return m_Manager; }
	void UpdateByManager() { m_Manager->UpdateBuffer(this); }

private:
	int m_Stream;
	VertexFormat m_Format;

	BufferManager* m_Manager;
	void* m_Handle;
};

}
}

#endif // #ifndef INCLUDED_LUX_VERTEX_BUFFER_IMPL_H
