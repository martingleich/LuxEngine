#include "MemoryFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"

namespace lux
{
namespace io
{

MemoryFile::MemoryFile(void* Buffer,
	const FileDescription& desc,
	string name,
	bool DeleteBufferOnDrop,
	bool Expandable) :
	File(name, desc),
	m_Buffer((u8*)Buffer),
	m_Size(desc.GetSize()),
	m_Cursor(0),
	m_IsEOF(false),
	m_DeleteBufferOnDrop(DeleteBufferOnDrop),
	m_IsExpandable(Expandable)
{
}

MemoryFile::~MemoryFile()
{
	if(m_Buffer && m_DeleteBufferOnDrop)
		LUX_FREE_ARRAY(m_Buffer);
}

u32 MemoryFile::ReadBinary(u32 numBytes, void* out)
{
	if(numBytes == 0 || !out)
		return 0;

	if(m_Cursor + numBytes > m_Size) {
		numBytes = m_Size - m_Cursor;
		m_IsEOF = true;
	}

	memcpy(out, m_Buffer + m_Cursor, numBytes);
	m_Cursor += numBytes;

	return numBytes;
}

u32 MemoryFile::WriteBinary(const void* data, u32 numBytes)
{
	if(numBytes == 0 || !data)
		return 0;

	if(m_Cursor > m_Size - numBytes) {
		if(!m_IsExpandable) {
			throw core::FileException(core::FileException::WriteError);
		} else {
			u8* pNewData = LUX_NEW_ARRAY(u8, ((m_Cursor + numBytes) * 3) / 2);

			if(m_Buffer) {
				memcpy(pNewData, m_Buffer, m_Size);
				LUX_FREE_ARRAY(m_Buffer);
			}

			m_Buffer = pNewData;
			m_Size = (3 * (m_Cursor + numBytes)) / 2;
		}
	}

	memcpy(m_Buffer + m_Cursor, data, numBytes);
	m_Cursor += numBytes;

	return numBytes;
}

bool MemoryFile::Seek(s32 offset, ESeekOrigin origin)
{
	if(GetSize() == 0)
		return false;

	u32 cursor;
	switch(origin) {
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
		m_Cursor = newCursor;
		m_IsEOF = (m_Cursor == m_Size);
		return true;
	} else {
		log::Error("Readcursor was moved outside the file.");
		return false;
	}
}


void* MemoryFile::GetBuffer()
{
	return m_Buffer;
}

const void* MemoryFile::GetBuffer() const
{
	return m_Buffer;
}

u32 MemoryFile::GetSize() const
{
	return m_Size;
}

u32 MemoryFile::GetCursor() const
{
	return m_Cursor;
}

}

}



