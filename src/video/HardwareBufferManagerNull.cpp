#include "HardwareBufferManagerNull.h"
#include "video/IndexBufferImpl.h"
#include "video/VertexBufferImpl.h"

namespace lux
{
namespace video
{

BufferManagerNull::BufferManagerNull(VideoDriver* driver) :
	m_Driver(driver)
{
}

VideoDriver* BufferManagerNull::GetDriver()
{
	return m_Driver;
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
	return LUX_NEW(IndexBufferImpl)(this);
}

StrongRef<VertexBuffer> BufferManagerNull::CreateVertexBuffer()
{
	return LUX_NEW(VertexBufferImpl)(this);
}

void BufferManagerNull::EnableBuffer(const HardwareBuffer* buffer, int streamID)
{
	if(!buffer)
		return;
	void* handle = buffer->GetHandle();
	EnableHardwareBuffer(streamID, buffer, handle);
}

} // namespace video
} // namespace lux