#include "video/HardwareBuffer.h"
#include "math/lxMath.h"

namespace lux
{
namespace video
{

EHardwareBufferType HardwareBuffer::GetBufferType() const
{
	return m_BufferType;
}

HardwareBuffer::HardwareBuffer(BufferManager* mgr, EHardwareBufferType type) :
	m_Manager(mgr),
	m_BufferType(type)
{
	m_Manager->AddBuffer(this);
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
	m_Allocated = 0;
	m_Mapping = EHardwareBufferMapping::Static;
	m_Handle = nullptr;
}

 HardwareBuffer::~HardwareBuffer()
{
	m_Manager->RemoveBuffer(this);
	Clear();
}

 void HardwareBuffer::Clear()
{
	LUX_FREE_ARRAY(m_Data);
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
	m_Allocated = 0;
}

void HardwareBuffer::ResetDirty()
{
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
}

void* HardwareBuffer::GetHandle() const
{
	return m_Handle;
}

void* HardwareBuffer::Pointer(u32 n, u32 count)
{
	assert(n + count <= m_Size);
	m_BeginDirty = math::Min(m_BeginDirty, n);
	m_EndDirty = math::Max(m_EndDirty, n + count - 1);
	return m_Data + n*m_Stride;
}

const void* HardwareBuffer::Pointer(u32 n, u32 count) const
{
	assert(n + count <= m_Size);
	return m_Data + n*m_Stride;
}

const void* HardwareBuffer::Pointer_c(u32 n, u32 count) const
{
	return this->Pointer(n, count);
}

u32 HardwareBuffer::GetSize() const
{
	return m_Size;
}

u32 HardwareBuffer::GetAlloc() const
{
	return m_Allocated;
}

bool HardwareBuffer::Update(u32 group)
{
	if(m_EndDirty < m_BeginDirty)
		return true;

	if(!m_Manager->UpdateBuffer(this, group))
		return false;

	m_EndDirty = 0;
	m_BeginDirty = 0xFFFFFFFF;

	return true;
}

void HardwareBuffer::SetCursor(u32 c)
{
	assert(c < m_Size);
	m_Cursor = c;
}

u32 HardwareBuffer::GetCursor() const
{
	return m_Cursor;
}

u32 HardwareBuffer::GetStride() const
{
	return m_Stride;
}

void HardwareBuffer::SetHWMapping(EHardwareBufferMapping hwm)
{
	m_Mapping = hwm;
}

EHardwareBufferMapping HardwareBuffer::GetHWMapping() const
{
	return m_Mapping;
}

bool HardwareBuffer::GetDirty(u32& begin, u32& end) const
{
	if(m_EndDirty >= m_BeginDirty) {
		begin = m_BeginDirty;
		end = m_EndDirty;
		return true;
	}
	return false;
}

void HardwareBuffer::Reserve(u32 size, bool moveOld, void* init)
{
	if(size <= m_Allocated)
		return;

	u8* old = m_Data;
	m_Data = LUX_NEW_ARRAY(u8, size*m_Stride);

	if(moveOld && old) {
		memcpy(m_Data, old, m_Size*m_Stride);
		if(init)
			for(u32 i = m_Size; i < size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
	} else {
		if(init)
			for(u32 i = m_Size; i < size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
	}

	LUX_FREE_ARRAY(old);

	m_Allocated = size;
}

void HardwareBuffer::SetSize(u32 size, void* init)
{
	if(size > m_Allocated)
		Reserve(size, true, init);

	if(size < m_Size) {
		if(m_Cursor >= size)
			m_Cursor = size - 1;

		if(m_EndDirty >= size)
			m_EndDirty = size - 1;
	} else {
		m_BeginDirty = math::Min(m_BeginDirty, m_Size - 1);
		m_EndDirty = size - 1;
	}

	m_Size = size;
}

BufferManager* HardwareBuffer::GetManager()
{
	return m_Manager;
}

}
}
