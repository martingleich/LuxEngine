#ifndef INCLUDED_HARDWAREBUFFER_H
#define INCLUDED_HARDWAREBUFFER_H
#include "video/HardwareBufferManager.h"
#include "HardwareBufferConstants.h"

namespace lux
{
namespace video
{

class LUX_API HardwareBuffer : public ReferenceCounted
{
	friend class BufferManagerNull;
public:
	EHardwareBufferType GetBufferType() const;

	HardwareBuffer(BufferManager* mgr, EHardwareBufferType type);
	virtual ~HardwareBuffer();

	virtual void Clear();
	void ResetDirty();
	void* GetHandle() const;
	void* Pointer(u32 n, u32 count);
	const void* Pointer(u32 n, u32 count) const;
	const void* Pointer_c(u32 n, u32 count) const;
	u32 GetSize() const;
	u32 GetAlloc() const;
	bool Update(u32 group = 0);
	void SetCursor(u32 c);
	u32 GetCursor() const;
	u32 GetStride() const;
	void SetHWMapping(EHardwareBufferMapping hwm);
	EHardwareBufferMapping GetHWMapping() const;
	bool GetDirty(u32& begin, u32& end) const;
	void Reserve(u32 size, bool moveOld = true, void* init = nullptr);
	void SetSize(u32 size, void* init = nullptr);
	BufferManager* GetManager();

private:
	void* m_Handle;

protected:
	BufferManager* m_Manager;

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

}
}

#endif // !INCLUDED_HARDWAREBUFFER_H
