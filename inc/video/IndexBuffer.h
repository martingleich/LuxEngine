#ifndef INCLUDED_IINDEXBUFFER_H
#define INCLUDED_IINDEXBUFFER_H
#include "video/HardwareBuffer.h"

namespace lux
{
namespace video
{

class IndexBuffer : public HardwareBuffer
{
public:
	IndexBuffer(BufferManager* mgr) : HardwareBuffer(mgr, EHardwareBufferType::Index)
	{
	}
	virtual EIndexFormat GetType() const = 0;
	virtual void SetType(EIndexFormat type, bool moveOld = true, void* init = nullptr) = 0;
	virtual u32 AddIndex(void* index) = 0;
	virtual u32 AddIndices(void* indices, u32 count) = 0;
	virtual void SetIndex(void* index, u32 n) = 0;
	virtual void SetIndices(void* indices, u32 count, u32 n) = 0;
	virtual void GetIndex(void* ptr, u32 n) const = 0;
	virtual void GetIndices(void* ptr, u32 count, u32 n) const = 0;
	u32 GetIndex(u32 n) const
	{
		if(GetType() == EIndexFormat::Bit32) {
			u32 data;
			GetIndex(&data, n);
			return data;
		} else if(GetType() == EIndexFormat::Bit16) {
			u16 data;
			GetIndex(&data, n);
			return data;
		}
		assertNeverReach("Unknown index format.");
		return 0;
	}
};

class IndexBufferImpl : public IndexBuffer
{
	friend class BufferManagerNull;

private:
	EIndexFormat m_Type;

private:
	static u32 CalcStride(EIndexFormat type);

	IndexBufferImpl(BufferManager* mgr);

public:
	EIndexFormat GetType() const;
	void SetType(EIndexFormat type, bool moveOld = true, void* init = nullptr);
	u32 AddIndex(void* index);
	u32 AddIndices(void* indices, u32 count);
	void SetIndex(void* index, u32 n);
	void SetIndices(void* indices, u32 count, u32 n);
	void GetIndex(void* ptr, u32 n) const;
	void GetIndices(void* ptr, u32 count, u32 n) const;
};

}    //namespace video
}    //namespace lux

#endif