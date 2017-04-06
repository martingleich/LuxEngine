#include "LimitedFile.h"
#include "core/Logger.h"

namespace lux
{
namespace io
{

LimitedFile::LimitedFile(StrongRef<File> Master,
	u32 offset,
	const FileDescription& desc,
	string name) :
	File(name, desc),
	m_MasterFile(Master),
	m_StartOffset(offset),
	m_FileSize(desc.GetSize()),
	m_Cursor(0)
{
}

u32 LimitedFile::ReadBinary(u32 numBytes, void* out)
{
	u32 avail = math::Min(m_FileSize - m_Cursor - 1, numBytes);
	if(numBytes > avail)
		numBytes = avail;
	u32 readBytes = m_MasterFile->ReadBinary(numBytes, out);
	m_Cursor += readBytes;
	return readBytes;
}

u32 LimitedFile::WriteBinary(const void* data, u32 length)
{
	u32 written = m_MasterFile->WriteBinary(data, length);
	m_Cursor += written;
	return written;
}

bool LimitedFile::Seek(s32 offset, ESeekOrigin orgin)
{
	if(GetSize() == 0)
		return false;

	u32 cursor;
	switch(orgin) {
	case ESeekOrigin::Start:
		cursor = 0;
		break;
	case ESeekOrigin::End:
		cursor = GetSize();
		break;
	case ESeekOrigin::Cursor:
		cursor = GetCursor();
		break;
	default:
		cursor = 0;
	}

	u32 newCursor;
	bool success = math::AddInsideBounds(cursor, offset, GetSize(), newCursor);

	if(success) {
		return m_MasterFile->Seek(newCursor, ESeekOrigin::Start);
	} else {
		log::Error("Readcursor was moved outside of file.");
		return false;
	}
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

