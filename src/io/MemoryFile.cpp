#include "MemoryFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"

namespace lux
{
namespace io
{

MemoryFile::MemoryFile(void* buffer,
	const FileDescription& desc,
	const core::String& name,
	EVirtualCreateFlag flags) :
	File(name, desc),
	m_Buffer((u8*)buffer),
	m_Cursor(0),
	m_IsEOF(false),
	m_Flags(flags)
{
	if(desc.GetSize() > std::numeric_limits<size_t>::max())
		throw core::Exception("Memory file is to big");
	m_Size = (size_t)desc.GetSize();

	if(TestFlag(m_Flags, EVirtualCreateFlag::Copy)) {
		auto newData = LUX_NEW_RAW(m_Size);
		if(m_Buffer)
			memcpy(newData, m_Buffer, m_Size);
		if(TestFlag(m_Flags, EVirtualCreateFlag::DeleteOnDrop))
			LUX_FREE_RAW(m_Buffer);

		m_Buffer = newData;
		SetFlag(m_Flags, EVirtualCreateFlag::DeleteOnDrop | EVirtualCreateFlag::Expandable);
		ClearFlag(m_Flags, EVirtualCreateFlag::Copy | EVirtualCreateFlag::ReadOnly);
	}
}

MemoryFile::~MemoryFile()
{
	if(TestFlag(m_Flags, EVirtualCreateFlag::DeleteOnDrop))
		LUX_FREE_RAW(m_Buffer);
}

s64 MemoryFile::ReadBinaryPart(s64 numBytes, void* out)
{
	LX_CHECK_NULL_ARG(out);
	if(numBytes <= 0 || numBytes > std::numeric_limits<size_t>::max())
		throw io::FileException(io::FileException::ReadError);
	size_t sizeBytes = (size_t)numBytes;

	if(m_Cursor + sizeBytes > m_Size) {
		sizeBytes = m_Size - m_Cursor;
		m_IsEOF = true;
	}

	memcpy(out, m_Buffer + m_Cursor, sizeBytes);
	m_Cursor += sizeBytes;

	return (s64)sizeBytes;
}

s64 MemoryFile::WriteBinaryPart(const void* data, s64 numBytes)
{
	LX_CHECK_NULL_ARG(data);
	if(numBytes < 0 || numBytes > std::numeric_limits<size_t>::max())
		throw io::FileException(io::FileException::WriteError);
	size_t sizeBytes = (size_t)numBytes;

	if(TestFlag(m_Flags, EVirtualCreateFlag::ReadOnly))
		throw io::FileException(io::FileException::WriteError);

	if(m_Cursor > m_Size - sizeBytes) {
		if(!TestFlag(m_Flags, EVirtualCreateFlag::Expandable)) {
			throw io::FileException(io::FileException::WriteError);
		} else {
			u8* pNewData = LUX_NEW_RAW(((m_Cursor + sizeBytes) * 3) / 2);
			if(m_Buffer)
				memcpy(pNewData, m_Buffer, m_Size);
			if(TestFlag(m_Flags, EVirtualCreateFlag::DeleteOnDrop))
				LUX_FREE_RAW(m_Buffer);
			SetFlag(m_Flags, EVirtualCreateFlag::DeleteOnDrop | EVirtualCreateFlag::Expandable);
			ClearFlag(m_Flags, EVirtualCreateFlag::Copy | EVirtualCreateFlag::ReadOnly);

			m_Buffer = pNewData;
			m_Size = (3 * (m_Cursor + sizeBytes)) / 2;
		}
	}

	memcpy(m_Buffer + m_Cursor, data, sizeBytes);
	m_Cursor += sizeBytes;

	return (s64)sizeBytes;
}

void MemoryFile::Seek(s64 offset, ESeekOrigin origin)
{
	size_t cursor = (origin == ESeekOrigin::Start) ? 0 : (size_t)GetCursor();

	size_t newCursor = (size_t)((s64)cursor + offset);
	if(newCursor > GetSize())
		throw io::FileException(io::FileException::OutsideFile);

	m_Cursor = newCursor;
	m_IsEOF = (m_Cursor == m_Size);
}


void* MemoryFile::GetBuffer()
{
	if(TestFlag(m_Flags, EVirtualCreateFlag::ReadOnly))
		return nullptr;
	else
		return m_Buffer;
}

const void* MemoryFile::GetBuffer() const
{
	return m_Buffer;
}

s64 MemoryFile::GetSize() const
{
	return (s64)m_Size;
}

s64 MemoryFile::GetCursor() const
{
	return (s64)m_Cursor;
}

}
}
