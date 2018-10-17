#include "StreamFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"
#include "math/lxMath.h"

namespace lux
{
namespace io
{

StreamFileWin32::StreamFileWin32(
	Win32FileHandle file,
	const FileInfo& info,
	const Path& path) :
	m_Path(path),
	m_File(std::move(file)),
	m_Info(info),
	m_Cursor(0)
{
}

StreamFileWin32::~StreamFileWin32()
{
}

s64 StreamFileWin32::ReadBinaryPart(s64 numBytes, void* out)
{
	LX_CHECK_NULL_ARG(out);
	LX_CHECK_NULL_ARG(numBytes);
	DWORD numBytesDWORD;
	if(!core::CheckedCast(numBytes, numBytesDWORD))
		throw io::FileUsageException(io::FileUsageException::ReadError, GetPath());

	DWORD read;
	BOOL result = ReadFile(m_File, out, numBytesDWORD, &read, NULL);
	if(!result)
		throw io::FileUsageException(io::FileUsageException::ReadError, GetPath());
	m_Cursor += read;

	return read;
}

s64 StreamFileWin32::WriteBinaryPart(const void* data, s64 numBytes)
{
	LX_CHECK_NULL_ARG(data);
	LX_CHECK_NULL_ARG(numBytes);

	DWORD numBytesDWORD;
	if(!core::CheckedCast(numBytes, numBytesDWORD))
		throw io::FileUsageException(io::FileUsageException::ReadError, GetPath());

	DWORD written;
	BOOL result = WriteFile(m_File, data, numBytesDWORD, &written, NULL);
	if(!result && written != numBytesDWORD)
		throw io::FileUsageException(io::FileUsageException::WriteError, GetPath());

	m_Cursor += written;
	if(m_Cursor > GetSize())
		m_Info.SetSize(m_Cursor);
	return written;
}

void StreamFileWin32::Seek(s64 offset, ESeekOrigin origin)
{
	s64 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();
	s64 newCursor = cursor + offset;
	s64 cursorOffset = newCursor - GetCursor();
	if(newCursor < 0 || newCursor >= GetSize())
		throw io::FileUsageException(io::FileUsageException::CursorOutsideFile, GetPath());

	LARGE_INTEGER lint;
	lint.QuadPart = core::SafeCast<LONGLONG>(cursorOffset);
	BOOL result = SetFilePointerEx(m_File, lint, NULL, FILE_CURRENT);
	if(!result)
		throw core::GenericRuntimeException("Seeking failed");
	m_Cursor = newCursor;
}

void* StreamFileWin32::GetBuffer()
{
	return nullptr;
}

const void* StreamFileWin32::GetBuffer() const
{
	return nullptr;
}

s64 StreamFileWin32::GetSize() const
{
	return m_Info.GetSize();
}

s64 StreamFileWin32::GetCursor() const
{
	return m_Cursor;
}

bool StreamFileWin32::IsEOF() const
{
	return m_Cursor == GetSize();
}

}
}
