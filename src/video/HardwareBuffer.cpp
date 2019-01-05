#include "video/HardwareBuffer.h"
#include "video/HardwareBufferManager.h"

namespace lux
{
namespace video
{

HardwareBuffer::HardwareBuffer(BufferManager* mgr, EHardwareBufferType type) :
	m_BufferType(type),
	m_Manager(mgr)
{
	m_Manager->AddBuffer(this);

	m_ChangeId = 0;
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = INT_MAX;
	m_EndDirty = 0;
	m_Allocated = 0;
	m_Mapping = EHardwareBufferMapping::Static;
}

HardwareBuffer::~HardwareBuffer()
{
	Clear();
	m_Manager->RemoveBuffer(this);
}

void HardwareBuffer::Clear()
{
	LUX_FREE_ARRAY(m_Data);
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = INT_MAX;
	m_EndDirty = 0;
	m_Allocated = 0;
}
void HardwareBuffer::UpdateByManager()
{
	m_Manager->UpdateBuffer(this);
}

bool HardwareBuffer::GetDirty(int& begin, int& end) const
{
	if(m_EndDirty >= m_BeginDirty) {
		begin = m_BeginDirty;
		end = m_EndDirty;
		return true;
	}
	return false;
}

void HardwareBuffer::Reserve(int size, bool moveOld, void* init)
{
	if(size <= m_Allocated)
		return;

	u8* old = m_Data;
	m_Data = LUX_NEW_ARRAY(u8, size*m_Stride);

	if(moveOld && old) {
		memcpy(m_Data, old, m_Size*m_Stride);
		if(init)
			for(int i = m_Size; i < size; ++i)
				memcpy(m_Data + i * m_Stride, init, m_Stride);
	} else {
		if(init)
			for(int i = m_Size; i < size; ++i)
				memcpy(m_Data + i * m_Stride, init, m_Stride);
	}

	LUX_FREE_ARRAY(old);

	m_Allocated = size;
}

void HardwareBuffer::SetSize(int size, bool moveOld, void* init)
{
	if(size > m_Allocated)
		Reserve(size, moveOld, init);

	if(size < m_Size) {
		if(m_Cursor >= size)
			m_Cursor = size - 1;

		if(m_BeginDirty > m_EndDirty)
			m_BeginDirty = 0;
		if(m_EndDirty >= size)
			m_EndDirty = size - 1;
	} else {
		if(m_BeginDirty > m_EndDirty)
			m_BeginDirty = 0;
		if(m_BeginDirty >= m_Size)
			m_BeginDirty = m_Size > 0 ? m_Size - 1 : INT_MAX;
		m_EndDirty = size > 0 ? size - 1 : 0;
	}

	m_Size = size;
	++m_ChangeId;
}

void* HardwareBuffer::Pointer(int n, int count)
{
	lxAssert(n + count <= m_Size);
	m_BeginDirty = math::Min(m_BeginDirty, n);
	m_EndDirty = math::Max(m_EndDirty, n + count - 1);
	m_ChangeId++;
	return m_Data + n * m_Stride;
}

const void* HardwareBuffer::Pointer(int n, int count) const
{
	LUX_UNUSED(count);
	lxAssert(n + count <= m_Size);
	return m_Data + n * m_Stride;
}

void HardwareBuffer::SetDirty(int begin, int end)
{
	if(begin > m_Size)
		ResetDirty();
	else if(end > m_Size)
		m_EndDirty = math::Max(m_Size - 1, 0);
	else {
		m_BeginDirty = begin;
		m_EndDirty = end;
	}
}

void HardwareBuffer::Update()
{
	if(m_EndDirty < m_BeginDirty)
		return;

	UpdateByManager();

	m_EndDirty = 0;
	m_BeginDirty = INT_MAX;
}

} // namespace video
} // namespace lux