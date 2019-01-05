#ifndef INCLUDED_LUX_IINDEXBUFFER_H
#define INCLUDED_LUX_IINDEXBUFFER_H
#include "video/HardwareBuffer.h"

namespace lux
{
namespace video
{

class IndexBuffer : public HardwareBuffer
{
public:
	LUX_API IndexBuffer(BufferManager* mgr);
	LUX_API ~IndexBuffer();

	LUX_API void SetFormat(EIndexFormat type, bool moveOld = true, void* init = nullptr);
	LUX_API EIndexFormat GetFormat() const { return m_Format; }

	LUX_API int AddIndex(const void* index);
	LUX_API int AddIndices(const void* indices, int count);
	LUX_API int AddIndices32(const u32* indices, int count);
	LUX_API void SetIndex(const void* index, int n);
	LUX_API void SetIndices(const void* indices, int count, int n);
	LUX_API void SetIndices32(const u32* indices, int count, int n);
	LUX_API void GetIndex(void* ptr, int n) const;
	LUX_API void GetIndices(void* ptr, int count, int n) const;

	LUX_API int GetIndex(int n) const;

private:
	EIndexFormat m_Format;
};

}    //namespace video
}    //namespace lux

#endif