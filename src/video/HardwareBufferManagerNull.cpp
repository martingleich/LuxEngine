#include "HardwareBufferManagerNull.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

namespace lux
{
namespace video
{

BufferManagerNull::BufferManagerNull(VideoDriver* driver) :
	m_Driver(driver)
{
}

void BufferManagerNull::AddBuffer(HardwareBuffer* buffer)
{
	buffer->SetHandle(nullptr);
}

void BufferManagerNull::RemoveBuffer(HardwareBuffer* buffer)
{
	RemoveInternalBuffer(buffer, buffer->GetHandle());
	buffer->SetHandle(nullptr);
}

void BufferManagerNull::UpdateBuffer(HardwareBuffer* buffer)
{
	if(buffer->GetSize() == 0)
		return;

	ForceBufferUpdate(buffer);
}

void BufferManagerNull::ForceBufferUpdate(HardwareBuffer* buffer)
{
	auto handle = buffer->GetHandle();
	handle = UpdateInternalBuffer(buffer, handle);

	buffer->SetHandle(handle);
}

StrongRef<IndexBuffer> BufferManagerNull::CreateIndexBuffer()
{
	return LUX_NEW(IndexBuffer)(this);
}

StrongRef<VertexBuffer> BufferManagerNull::CreateVertexBuffer()
{
	return LUX_NEW(VertexBuffer)(this);
}

void BufferManagerNull::EnableBuffer(const HardwareBuffer* buffer)
{
	if(!buffer)
		return;
	void* handle = buffer->GetHandle();
	EnableHardwareBuffer(buffer, handle);
}

} // namespace video
} // namespace lux