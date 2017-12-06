#ifndef INCLUDED_LIMITEDFILE_H
#define INCLUDED_LIMITEDFILE_H
#include "io/File.h"

namespace lux
{
namespace io
{

class LimitedFile : public File
{
public:
	LimitedFile(
		StrongRef<File> Master,
		u32 offset,
		const FileDescription& desc,
		core::String name);

	u32 ReadBinaryPart(u32 numBytes, void* out);
	u32 WriteBinaryPart(const void* data, u32 length);
	void Seek(u32 offset, ESeekOrigin origin);
	void* GetBuffer();
	const void* GetBuffer() const;
	u32 GetSize() const;
	u32 GetCursor() const;

private:
	u32 m_StartOffset;
	u32 m_FileSize;
	u32 m_Cursor;
	StrongRef<File> m_MasterFile;

};

} //namespace io
} //namespace lux

#endif