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
	IndexBuffer() : HardwareBuffer(EHardwareBufferType::Index) {}

	virtual EIndexFormat GetType() const = 0;
	virtual void SetType(EIndexFormat type, bool moveOld = true, void* init = nullptr) = 0;
	virtual u32 AddIndex(void* index) = 0;
	virtual u32 AddIndices(void* indices, u32 count) = 0;
	virtual void SetIndex(void* index, u32 n) = 0;
	virtual void SetIndices(void* indices, u32 count, u32 n) = 0;
	virtual void GetIndex(void* ptr, u32 n) const = 0;
	virtual void GetIndices(void* ptr, u32 count, u32 n) const = 0;
	virtual u32 GetIndex(u32 n) const = 0;
};

}    //namespace video
}    //namespace lux

#endif