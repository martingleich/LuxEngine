#include "HardwareBufferManagerNull.h"
#include "video/HardwareBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"

namespace lux
{
namespace video
{


BufferManagerNull::UpdateEntry::UpdateEntry(u32 g, HardwareBuffer* b) : group(g), buffer(b)
{
}
BufferManagerNull::UpdateEntry::UpdateEntry() : group(0), buffer(nullptr)
{
}
BufferManagerNull::UpdateEntry::UpdateEntry(UpdateEntry&& old)
{
	group = old.group;
	buffer = old.buffer;
	if(buffer->m_Handle == &old)
		buffer->m_Handle = this;
}

BufferManagerNull::UpdateEntry& BufferManagerNull::UpdateEntry::operator=(const UpdateEntry&& old)
{
	group = old.group;
	buffer = old.buffer;
	if(buffer->m_Handle == &old)
		buffer->m_Handle = this;

	return *this;
}

BufferManagerNull::BufferManagerNull(VideoDriver* driver)
{
	m_Driver = driver;
	m_MinHardwareBufferBytes = 0;
}

void BufferManagerNull::SetMinHardwareBufferBytes(u32 bytes)
{
	m_MinHardwareBufferBytes = bytes;
}

u32 BufferManagerNull::GetMinHardwareBufferBytes() const
{
	return m_MinHardwareBufferBytes;
}

VideoDriver* BufferManagerNull::GetDriver()
{
	return m_Driver;
}
void BufferManagerNull::Update(u32 updateGroup)
{
	for(auto it = m_Updates.First(); it != m_Updates.End();)
		if(it->group == updateGroup) {
			ForceBufferUpdate(it->buffer);
			m_Updates.Erase(it);
		} else {
			++it;
		}
}

void BufferManagerNull::AddBuffer(HardwareBuffer* buffer)
{
	LUX_UNUSED(buffer);
}

void BufferManagerNull::RemoveBuffer(HardwareBuffer* buffer)
{
	for(auto it = m_Updates.First(); it != m_Updates.End();) {
		if(it->buffer == buffer)
			m_Updates.Erase(it);
		else
			++it;
	}

	RemoveInternalBuffer(buffer, buffer->m_Handle);
	buffer->m_Handle = nullptr;
}

bool BufferManagerNull::UpdateBuffer(HardwareBuffer* buffer, u32 group)
{
	if(group != 0) {
		m_Updates.Push_Back(UpdateEntry(group, buffer));
		buffer->m_Handle = m_Updates.Data() + m_Updates.Size() - 1;
		return true;
	} else {
		return ForceBufferUpdate(buffer);
	}
}

bool BufferManagerNull::ShouldCreateHardwareBuffer(HardwareBuffer* buffer)
{
	if(buffer->GetHWMapping() == EHardwareBufferMapping::Never)
		return false;
	if(buffer->GetSize() * buffer->GetStride() < GetMinHardwareBufferBytes())
		return false;

	return true;
}
bool BufferManagerNull::ForceBufferUpdate(HardwareBuffer* buffer)
{
	if(!ShouldCreateHardwareBuffer(buffer))
		return true;

	void*& handle = buffer->m_Handle;
	if(!handle && handle > m_Updates.Data() && handle < m_Updates.Data() + m_Updates.Size() - 1)
		handle = nullptr;

	handle = UpdateInternalBuffer(buffer, handle);

	if(handle == nullptr)
		return false;
	else
		return true;
}

IndexBuffer* BufferManagerNull::CreateIndexBuffer()
{
	return LUX_NEW(IndexBufferImpl)(this);
}

VertexBuffer* BufferManagerNull::CreateVertexBuffer()
{
	return LUX_NEW(VertexBufferImpl)(this);
}

bool BufferManagerNull::EnableBuffer(const HardwareBuffer* buffer, u32 streamID)
{
	if(m_Updates.Size() > 0 && buffer->m_Handle > m_Updates.Data() && buffer->m_Handle < m_Updates.Data() + m_Updates.Size() - 1)
		return EnableHardwareBuffer(streamID, buffer, nullptr);
	else
		return EnableHardwareBuffer(streamID, buffer, buffer->m_Handle);
}

}
}