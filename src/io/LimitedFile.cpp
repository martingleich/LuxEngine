#include "LimitedFile.h"
#include "core/Logger.h"

namespace lux
{
namespace io
{

LimitedFile::LimitedFile(StrongRef<File> Master,
	u32 offset,
	const FileDescription& desc,
	core::String name) :
	File(name, desc),
	m_StartOffset(offset),
	m_FileSize(desc.GetSize()),
	m_Cursor(0),
	m_MasterFile(Master)
{
}

u32 LimitedFile::ReadBinaryPart(u32 numBytes, void* out)
{
	u32 avail = math::Min(m_FileSize - m_Cursor - 1, numBytes);
	if(numBytes > avail)
		numBytes = avail;
	u32 readBytes = m_MasterFile->ReadBinaryPart(numBytes, out);
	m_Cursor += readBytes;
	return readBytes;
}

u32 LimitedFile::WriteBinaryPart(const void* data, u32 length)
{
	u32 written = m_MasterFile->WriteBinaryPart(data, length);
	m_Cursor += written;
	return written;
}

void LimitedFile::Seek(u32 offset, ESeekOrigin origin)
{
	u32 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();

	u32 newCursor;
	bool success = math::AddInsideBounds(cursor, offset, GetSize(), newCursor);

	if(!success)
		throw core::FileException(core::FileException::OutsideFile);

	m_MasterFile->Seek(newCursor, ESeekOrigin::Start);
}

void* LimitedFile::GetBuffer()
{
	return nullptr;
}
const void* LimitedFile::GetBuffer() const
{
	return nullptr;
}
u32 LimitedFile::GetSize() const
{
	return m_FileSize;
}
u32 LimitedFile::GetCursor() const
{
	return m_Cursor;
}

}
}

