#include "StreamFile.h"
#include "core/Logger.h"
#include "core/lxMemory.h"
#include "math/lxMath.h"

namespace lux
{
namespace io
{

StreamFile::StreamFile(FILE* file,
	const FileDescription& desc,
	const string& name) :
	File(name, desc),
	m_File(file),
	m_FileSize(desc.GetSize())
{
}

StreamFile::~StreamFile()
{
	if(m_File)
		fclose(m_File);
}

u32 StreamFile::ReadBinary(u32 numBytes, void* out)
{
	if(numBytes == 0 || !out)
		return 0;

	if((u32)ftell(m_File) + numBytes > m_FileSize)
		numBytes = m_FileSize - ftell(m_File);

	u32 read = (u32)fread(out, numBytes, 1, m_File)*numBytes;
	if(read != numBytes) {
		u8* cur = (u8*)out + read;
		u32 count = numBytes - read;
		while(count > 0) {
			u32 block = (u32)fread(cur, 1024, 1, m_File) * 1024;
			read += block;
			if(block != 1024)
				return (u32)read;

			cur += block;
			count -= block;
		}
	}

	return (u32)read;
}

u32 StreamFile::WriteBinary(const void* data, u32 numBytes)
{
	if(numBytes == 0 || !data)
		return 0;

	if((u32)ftell(m_File) > m_FileSize - numBytes)
		m_FileSize = ftell(m_File) + numBytes;

	if(fwrite(data, numBytes, 1, m_File) != 1)
		throw core::FileException(core::FileException::WriteError);

	return numBytes;
}

bool StreamFile::Seek(s32 offset, ESeekOrigin orgin)
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
		fseek(m_File, newCursor, 0);
		return true;
	} else {
		log::Error("Readcursor was moved outside the file.");
		return false;
	}
}

void* StreamFile::GetBuffer()
{
	return nullptr;
}

const void* StreamFile::GetBuffer() const
{
	return nullptr;
}

u32 StreamFile::GetSize() const
{
	return m_FileSize;
}

u32 StreamFile::GetCursor() const
{
	return ftell(m_File);
}

}

}

