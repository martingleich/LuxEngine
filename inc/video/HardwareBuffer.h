#ifndef INCLUDED_HARDWAREBUFFER_H
#define INCLUDED_HARDWAREBUFFER_H
#include "core/ReferenceCounted.h"
#include "math/lxMath.h"

#include "video/VideoEnums.h"

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
	inline void SetDirty(int begin, int end);
	inline int GetChangeId() const;
	inline void* Pointer(int n, int count);
	inline const void* Pointer(int n, int count) const;
	inline const void* Pointer_c(int n, int count) const;
	inline int GetSize() const;
	inline int GetAlloc() const;
	inline void Update();
	inline void SetCursor(int c);
	inline int GetCursor() const;
	inline int GetStride() const;
	inline void SetHWMapping(EHardwareBufferMapping hwm);
	inline EHardwareBufferMapping GetHWMapping() const;
	inline bool GetDirty(int& begin, int& end) const;
	inline void Reserve(int size, bool moveOld = true, void* init = nullptr);
	inline void SetSize(int size, bool moveOld = true, void* init = nullptr);

	virtual void SetHandle(void* handle) = 0;
	virtual void* GetHandle() const = 0;
	virtual BufferManager* GetManager() = 0;
	virtual void UpdateByManager() = 0;

protected:
	u8* m_Data;
	int m_Size;
	int m_Allocated;

	int m_Cursor;

	int m_Stride;

	int m_BeginDirty; // First dirty element
	int m_EndDirty; // Last dirty element
	int m_ChangeId;

	EHardwareBufferMapping m_Mapping;
	EHardwareBufferType m_BufferType;
};

inline int HardwareBuffer::GetChangeId() const
{
	return m_ChangeId;
}

inline EHardwareBufferType HardwareBuffer::GetBufferType() const
{
	return m_BufferType;
}

inline HardwareBuffer::HardwareBuffer(EHardwareBufferType type) :
	m_BufferType(type)
{
	m_ChangeId = 0;
	m_Data = nullptr;
	m_Size = 0;
	m_Cursor = 0;
	m_BeginDirty = INT_MAX;
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
	m_BeginDirty = INT_MAX;
	m_EndDirty = 0;
	m_Allocated = 0;
}

inline void HardwareBuffer::ResetDirty()
{
	m_BeginDirty = INT_MAX;
	m_EndDirty = 0;
}

inline void HardwareBuffer::SetDirty(int begin, int end)
{
	m_BeginDirty = math::Min(m_Size-1, begin);
	m_EndDirty = math::Min(m_Size-1, end);
}

inline void* HardwareBuffer::Pointer(int n, int count)
{
	lxAssert(n + count <= m_Size);
	m_BeginDirty = math::Min(m_BeginDirty, n);
	m_EndDirty = math::Max(m_EndDirty, n + count - 1);
	m_ChangeId++;
	return m_Data + n*m_Stride;
}

inline const void* HardwareBuffer::Pointer(int n, int count) const
{
	LUX_UNUSED(count);
	lxAssert(n + count <= m_Size);
	return m_Data + n*m_Stride;
}

inline const void* HardwareBuffer::Pointer_c(int n, int count) const
{
	return this->Pointer(n, count);
}

inline int HardwareBuffer::GetSize() const
{
	return m_Size;
}

inline int HardwareBuffer::GetAlloc() const
{
	return m_Allocated;
}

inline void HardwareBuffer::Update()
{
	if(m_EndDirty < m_BeginDirty)
		return;

	UpdateByManager();

	m_EndDirty = 0;
	m_BeginDirty = INT_MAX;
}

inline void HardwareBuffer::SetCursor(int c)
{
	lxAssert(c <= m_Size);
	m_Cursor = c;
}

inline int HardwareBuffer::GetCursor() const
{
	return m_Cursor;
}

inline int HardwareBuffer::GetStride() const
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

inline bool HardwareBuffer::GetDirty(int& begin, int& end) const
{
	if(m_EndDirty >= m_BeginDirty) {
		begin = m_BeginDirty;
		end = m_EndDirty;
		return true;
	}
	return false;
}

inline void HardwareBuffer::Reserve(int size, bool moveOld, void* init)
{
	if(size <= m_Allocated)
		return;

	u8* old = m_Data;
	m_Data = LUX_NEW_ARRAY(u8, size*m_Stride);

	if(moveOld && old) {
		memcpy(m_Data, old, m_Size*m_Stride);
		if(init)
			for(int i = m_Size; i < size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
	} else {
		if(init)
			for(int i = m_Size; i < size; ++i)
				memcpy(m_Data + i*m_Stride, init, m_Stride);
	}

	LUX_FREE_ARRAY(old);

	m_Allocated = size;
}

inline void HardwareBuffer::SetSize(int size, bool moveOld, void* init)
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
		m_BeginDirty = math::Min(m_BeginDirty, m_Size - 1);
		m_EndDirty = size - 1;
	}

	m_Size = size;
	++m_ChangeId;
}

}
}

#endif // !INCLUDED_HARDWAREBUFFER_H
