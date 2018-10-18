#include "MemoryFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"

namespace lux
{
namespace io
{

MemoryFile::MemoryFile(
	void* buffer,
	const FileInfo& info,
	const Path& path,
	EVirtualCreateFlag flags) :
	m_Buffer((u8*)buffer),
	m_Info(info),
	m_Path(path),
	m_Cursor(0),
	m_IsEOF(false),
	m_Flags(flags)
{
	if(!core::CheckedCast(m_Info.GetSize(), m_Size))
		throw core::InvalidOperationException("Memory file is too big.");

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
	size_t sizeBytes;
	if(!core::CheckedCast(numBytes, sizeBytes))
		throw io::FileUsageException(io::FileUsageException::ReadError, GetPath());

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
	size_t sizeBytes;
	if(!core::CheckedCast(numBytes, sizeBytes))
		throw io::FileUsageException(io::FileUsageException::ReadError, GetPath());

	if(TestFlag(m_Flags, EVirtualCreateFlag::ReadOnly))
		throw io::FileUsageException(io::FileUsageException::WriteError, GetPath());

	if(m_Cursor > m_Size - sizeBytes) {
		if(!TestFlag(m_Flags, EVirtualCreateFlag::Expandable)) {
			throw io::FileUsageException(io::FileUsageException::WriteError, GetPath());
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
	s64 newCursor64 = (s64)cursor + offset;
	if(newCursor64 > GetSize())
		throw io::FileUsageException(io::FileUsageException::CursorOutsideFile, GetPath());

	size_t newCursor = core::SafeCast<size_t>(newCursor64);
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
	return m_Info.GetSize();
}

s64 MemoryFile::GetCursor() const
{
	return (s64)m_Cursor;
}

}
}
