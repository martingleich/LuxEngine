#include "LimitedFile.h"
#include "core/Logger.h"

namespace lux
{
namespace io
{

LimitedFile::LimitedFile(StrongRef<File> Master,
	s64 offset,
	const FileDescription& desc,
	core::String name) :
	File(name, desc),
	m_StartOffset(offset),
	m_FileSize(desc.GetSize()),
	m_Cursor(0),
	m_MasterFile(Master)
{
}

s64 LimitedFile::ReadBinaryPart(s64 numBytes, void* out)
{
	s64 avail = math::Min(m_FileSize - m_Cursor - 1, numBytes);
	if(numBytes > avail)
		numBytes = avail;
	s64 readBytes = m_MasterFile->ReadBinaryPart(numBytes, out);
	m_Cursor += readBytes;
	return readBytes;
}

s64 LimitedFile::WriteBinaryPart(const void* data, s64 length)
{
	s64 written = m_MasterFile->WriteBinaryPart(data, length);
	m_Cursor += written;
	return written;
}

void LimitedFile::Seek(s64 offset, ESeekOrigin origin)
{
	s64 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();

	s64 newCursor = cursor + offset;
	if(newCursor < 0 || newCursor > GetSize())
		throw io::FileException(io::FileException::OutsideFile);

	m_MasterFile->Seek(newCursor, ESeekOrigin::Start);
	m_Cursor = newCursor;
}

void* LimitedFile::GetBuffer()
{
	return nullptr;
}
const void* LimitedFile::GetBuffer() const
{
	return nullptr;
}
s64 LimitedFile::GetSize() const
{
	return m_FileSize;
}
s64 LimitedFile::GetCursor() const
{
	return m_Cursor;
}

}
}

