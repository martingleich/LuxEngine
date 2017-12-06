#ifndef INCLUDED_STREAMFILE_H
#define INCLUDED_STREAMFILE_H
#include "io/File.h"

namespace lux
{
namespace io
{

class StreamFile : public File
{
public:
	StreamFile(
		FILE* file,
		const FileDescription& desc,
		const core::String& name);

	~StreamFile();
	u32 ReadBinaryPart(u32 numBytes, void* out);
	u32 WriteBinaryPart(const void* data, u32 length);
	void Seek(u32 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	u32 GetSize() const;
	u32 GetCursor() const;

private:
	FILE* m_File;
	u32 m_FileSize;
};

} //namespace io
} //namespace lux

#endif