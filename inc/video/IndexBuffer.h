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

	virtual void SetFormat(EIndexFormat format, bool moveOld = true, void* init = nullptr) = 0;
	virtual EIndexFormat GetFormat() const = 0;

	virtual int AddIndex(const void* index) = 0;
	virtual int AddIndices(const void* indices, int count) = 0;
	virtual int AddIndices32(const u32* indices, int count) = 0;
	virtual void SetIndex(const void* index, int n) = 0;
	virtual void SetIndices(const void* indices, int count, int n) = 0;
	virtual void SetIndices32(const u32* indices, int count, int n) = 0;
	virtual void GetIndex(void* ptr, int n) const = 0;
	virtual void GetIndices(void* ptr, int count, int n) const = 0;
	virtual int GetIndex(int n) const = 0;
};

}    //namespace video
}    //namespace lux

#endif