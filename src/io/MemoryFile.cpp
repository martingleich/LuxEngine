#include "MemoryFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"

namespace lux
{
namespace io
{

MemoryFile::MemoryFile(void* Buffer,
	const FileDescription& desc,
	core::String name,
	bool DeleteBufferOnDrop,
	bool Expandable,
	bool readOnly) :
	File(name, desc),
	m_Buffer((u8*)Buffer),
	m_Size(desc.GetSize()),
	m_Cursor(0),
	m_IsEOF(false),
	m_DeleteBufferOnDrop(DeleteBufferOnDrop),
	m_IsExpandable(Expandable),
	m_IsReadOnly(readOnly)
{
}

MemoryFile::~MemoryFile()
{
	if(m_Buffer && m_DeleteBufferOnDrop)
		LUX_FREE_ARRAY(m_Buffer);
}

u32 MemoryFile::ReadBinaryPart(u32 numBytes, void* out)
{
	LX_CHECK_NULL_ARG(out);
	LX_CHECK_NULL_ARG(numBytes);

	if(m_Cursor + numBytes > m_Size) {
		numBytes = m_Size - m_Cursor;
		m_IsEOF = true;
	}

	memcpy(out, m_Buffer + m_Cursor, numBytes);
	m_Cursor += numBytes;

	return numBytes;
}

u32 MemoryFile::WriteBinaryPart(const void* data, u32 numBytes)
{
	LX_CHECK_NULL_ARG(data);
	LX_CHECK_NULL_ARG(numBytes);

	if(m_IsReadOnly)
		throw io::FileException(io::FileException::WriteError);

	if(numBytes == 0 || !data)
		return 0;

	if(m_Cursor > m_Size - numBytes) {
		if(!m_IsExpandable) {
			throw io::FileException(io::FileException::WriteError);
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

void MemoryFile::Seek(u32 offset, ESeekOrigin origin)
{
	u32 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();

	u32 newCursor;
	bool success = math::AddInsideBounds(cursor, offset, GetSize(), newCursor);

	if(!success)
		throw io::FileException(io::FileException::OutsideFile);

	m_Cursor = newCursor;
	m_IsEOF = (m_Cursor == m_Size);
}


void* MemoryFile::GetBuffer()
{
	if(m_IsReadOnly)
		return nullptr;
	else
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



