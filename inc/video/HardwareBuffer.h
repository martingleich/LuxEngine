#ifndef INCLUDED_LUX_HARDWAREBUFFER_H
#define INCLUDED_LUX_HARDWAREBUFFER_H
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
	LUX_API HardwareBuffer(BufferManager* mgr, EHardwareBufferType type);
	LUX_API ~HardwareBuffer();

	EHardwareBufferType GetBufferType() const { return m_BufferType; }
	LUX_API void Clear();
	LUX_API void SetDirty(int begin, int end);
	LUX_API void Update();

	void ResetDirty()
	{
		m_BeginDirty = INT_MAX;
		m_EndDirty = 0;
	}

	int GetChangeId() const { return m_ChangeId; }

	LUX_API const void* Pointer(int n, int count) const;
	LUX_API void* Pointer(int n, int count);

	void* Pointer() { return Pointer(0, m_Size); }
	const void* Pointer() const { return Pointer(0, m_Size); }

	const void* Pointer_c() const { return Pointer_c(0, m_Size); }
	const void* Pointer_c(int n, int count) const { return Pointer(n, count); }

	int GetSize() const { return m_Size; }
	int GetAlloc() const { return m_Allocated; }

	void SetCursor(int c) { lxAssert(c <= m_Size); m_Cursor = c; }

	int GetCursor() const { return m_Cursor; }
	int GetStride() const { return m_Stride; }
	void SetHWMapping(EHardwareBufferMapping hwm) { m_Mapping = hwm; }
	EHardwareBufferMapping GetHWMapping() const { return m_Mapping; }

	LUX_API bool GetDirty(int& begin, int& end) const;
	LUX_API void Reserve(int size, bool moveOld = true, void* init = nullptr);
	LUX_API void SetSize(int size, bool moveOld = true, void* init = nullptr);

	void SetHandle(void* handle) { m_Handle = handle; }
	void* GetHandle() const { return m_Handle; }
	BufferManager* GetManager() { return m_Manager; }

	LUX_API void UpdateByManager();

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

	BufferManager* m_Manager;
	void* m_Handle;
};


}
}

#endif // !INCLUDED_LUX_HARDWAREBUFFER_H
