#ifndef INCLUDED_LUX_LIMITEDFILE_H
#define INCLUDED_LUX_LIMITEDFILE_H
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
		s64 offset,
		const FileDescription& desc,
		core::String name);

	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;

private:
	s64 m_StartOffset;
	s64 m_FileSize;
	s64 m_Cursor;
	
	StrongRef<File> m_MasterFile;
};

} //namespace io
} //namespace lux

#endif