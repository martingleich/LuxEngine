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
		StrongRef<File> master,
		s64 offset,
		const FileInfo& info,
		const Path& path);

	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;
	const FileInfo& GetInfo() const { return m_Info; }
	const Path& GetPath() const { return m_Path; }

private:
	s64 m_StartOffset;
	FileInfo m_Info;
	Path m_Path;
	s64 m_Cursor;
	
	StrongRef<File> m_MasterFile;
};

} //namespace io
} //namespace lux

#endif