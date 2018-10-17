#ifndef INCLUDED_LUX_STREAMFILE_H
#define INCLUDED_LUX_STREAMFILE_H
#include "io/File.h"
#include "platform/WindowsUtils.h"

namespace lux
{
namespace io
{

class StreamFileWin32 : public File
{
public:
	StreamFileWin32(
		Win32FileHandle file,
		const FileInfo& info,
		const Path& path);

	~StreamFileWin32();
	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;
	bool IsEOF() const;

	const Path& GetPath() const { return m_Path; }
	const FileInfo& GetInfo() const { return m_Info; }

private:
	Path m_Path;
	Win32FileHandle m_File;
	FileInfo m_Info;
	s64 m_Cursor;
};

} //namespace io
} //namespace lux

#endif