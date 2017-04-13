#ifndef INCLUDED_HARDWAREBUFFER_H
#define INCLUDED_HARDWAREBUFFER_H
#include "core/ReferenceCounted.h"
#include "video/HardwareBufferConstants.h"
#include "math/lxMath.h"

namespace lux
{
namespace video
{
class BufferManager;

class HardwareBuffer : public ReferenceCounted
{
public:
	inline HardwareBuffer(EHardwareBufferType type);
	inline virtual ~HardwareBuffer();

	inline EHardwareBufferType GetBufferType() const;
	inline void Clear();
	inline void ResetDirty();
	inline void* Pointer(u32 n, u32 count);
	inline const void* Pointer(u32 n, u32 count) const;
	inline const void* Pointer_c(u32 n, u32 count) const;
	inline u32 GetSize() const;
	inline u32 GetAlloc() const;
	inline bool Update(u32 group = 0);
	inline void SetCursor(u32 c);
	inline u32 GetCursor() const;
	inline u32 GetStride() const;
	inline void SetHWMapping(EHardwareBufferMapping hwm);
	inline EHardwareBufferMapping GetHWMapping() const;
	inline bool GetDirty(u32& begin, u32& end) const;
	inline void Reserve(u32 size, bool moveOld = true, void* init = nullptr);
	inline void SetSize(u32 size, void* init = nullptr);

	virtual void SetHandle(void* handle) = 0;
	virtual void* GetHandle() const = 0;
	virtual BufferManager* GetManager() = 0;
	virtual bool UpdateByManager(u32 group) = 0;

protected:
	u8* m_Data;
	u32 m_Size;
	u32 m_Allocated;

	u32 m_Cursor;

	u32 m_Stride;

	u32 m_BeginDirty;
	u32 m_EndDirty;

	EHardwareBufferMapping m_Mapping;
	EHardwareBufferType m_BufferType;
};

inline EHardwareBufferType HardwareBuffer::GetBufferType() const
{
	return m_BufferType;
}

inline HardwareBuffer::HardwareBuffer(EHardwareBufferType type) :
	m_BufferType(type)
{
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
	m_Allocated = 0;
	m_Mapping = EHardwareBufferMapping::Static;
}

inline HardwareBuffer::~HardwareBuffer()
{
	Clear();
}

 inline void HardwareBuffer::Clear()
{
	LUX_FREE_ARRAY(m_Data);
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
	m_Allocated = 0;
}

inline void HardwareBuffer::ResetDirty()
{
	m_BeginDirty = 0xFFFFFFFF;
	m_EndDirty = 0;
}

inline void* HardwareBuffer::Pointer(u32 n, u32 count)
{
	assert(n + count <= m_Size);
	m_BeginDirty = math::Min(m_BeginDirty, n);
	m_EndDirty = math::Max(m_EndDirty, n + count - 1);
	return m_Data + n*m_Stride;
}

inline const void* HardwareBuffer::Pointer(u32 n, u32 count) const
{
	assert(n + count <= m_Size);
	return m_Data + n*m_Stride;
}

inline const void* HardwareBuffer::Pointer_c(u32 n, u32 count) const
{
	return this->Pointer(n, count);
}

inline u32 HardwareBuffer::GetSize() const
{
	return m_Size;
}

inline u32 HardwareBuffer::GetAlloc() const
{
	return m_Allocated;
}

inline bool HardwareBuffer::Update(u32 group)
{
	if(m_EndDirty < m_BeginDirty)
		return true;

	if(!UpdateByManager(group))
		return false;

	m_EndDirty = 0;
	m_BeginDirty = 0xFFFFFFFF;

	return true;
}

inline void HardwareBuffer::SetCursor(u32 c)
{
	assert(c < m_Size);
	m_Cursor = c;
}

inline u32 HardwareBuffer::GetCursor() const
{
	return m_Cursor;
}

inline u32 HardwareBuffer::GetStride() const
{
	return m_Stride;
}

inline void HardwareBuffer::SetHWMapping(EHardwareBufferMapping hwm)
{
	m_Mapping = hwm;
}

inline EHardwareBufferMapping HardwareBuffer::GetHWMapping() const
{
	return m_Mapping;
}

inline bool HardwareBuffer::GetDirty(u32& begin, u32& end) const
{
	if(m_EndDirty >= m_BeginDirty) {
		begin = m_BeginDirty;
		end = m_EndDirty;
		return true;
	}
	return false;
}

inline void HardwareBuffer::Reserve(u32 size, bool moveOld, void* init)
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

inline void HardwareBuffer::SetSize(u32 size, void* init)
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

}
}

#endif // !INCLUDED_HARDWAREBUFFER_H
