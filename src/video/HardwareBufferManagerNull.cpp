#include "HardwareBufferManagerNull.h"
#include "video/IndexBufferImpl.h"
#include "video/VertexBufferImpl.h"

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
	if(buffer->GetHandle() == &old)
		buffer->SetHandle(this);
}

BufferManagerNull::UpdateEntry& BufferManagerNull::UpdateEntry::operator=(const UpdateEntry&& old)
{
	group = old.group;
	buffer = old.buffer;
	if(buffer->GetHandle() == &old)
		buffer->SetHandle(this);

	return *this;
}

BufferManagerNull::BufferManagerNull(VideoDriver* driver) :
	m_Driver(driver)
{
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
	buffer->SetHandle(nullptr);
}

void BufferManagerNull::RemoveBuffer(HardwareBuffer* buffer)
{
	for(auto it = m_Updates.First(); it != m_Updates.End();) {
		if(it->buffer == buffer)
			m_Updates.Erase(it);
		else
			++it;
	}

	RemoveInternalBuffer(buffer, buffer->GetHandle());
	buffer->SetHandle(nullptr);
}

bool BufferManagerNull::UpdateBuffer(HardwareBuffer* buffer, u32 group)
{
	if(group != 0) {
		m_Updates.Push_Back(UpdateEntry(group, buffer));
		buffer->SetHandle(m_Updates.Data() + m_Updates.Size() - 1);
		return true;
	} else {
		return ForceBufferUpdate(buffer);
	}
}

bool BufferManagerNull::ForceBufferUpdate(HardwareBuffer* buffer)
{
	void* handle = buffer->GetHandle();
	if(!handle && handle > m_Updates.Data() && handle < m_Updates.Data() + m_Updates.Size() - 1)
		handle = nullptr;

	handle = UpdateInternalBuffer(buffer, handle);

	buffer->SetHandle(handle);

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
	void* handle = buffer->GetHandle();
	if(m_Updates.Size() > 0 && handle > m_Updates.Data() && handle < m_Updates.Data() + m_Updates.Size() - 1)
		return EnableHardwareBuffer(streamID, buffer, nullptr);
	else
		return EnableHardwareBuffer(streamID, buffer, handle);
}

}
}