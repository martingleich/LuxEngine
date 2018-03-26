#ifndef INCLUDED_LUX_STREAMFILE_H
#define INCLUDED_LUX_STREAMFILE_H
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
	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;
	bool IsEOF() const;

private:
	FILE* m_File;
	s64 m_FileSize;
};

} //namespace io
} //namespace lux

#endif