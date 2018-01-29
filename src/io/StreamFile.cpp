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
	const core::String& name) :
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

u32 StreamFile::ReadBinaryPart(u32 numBytes, void* out)
{
	LX_CHECK_NULL_ARG(out);
	LX_CHECK_NULL_ARG(numBytes);

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

u32 StreamFile::WriteBinaryPart(const void* data, u32 numBytes)
{
	LX_CHECK_NULL_ARG(data);
	LX_CHECK_NULL_ARG(numBytes);

	if((u32)ftell(m_File) > m_FileSize - numBytes)
		m_FileSize = ftell(m_File) + numBytes;

	if(fwrite(data, numBytes, 1, m_File) != 1)
		throw io::FileException(io::FileException::WriteError);

	return numBytes;
}

void StreamFile::Seek(u32 offset, ESeekOrigin origin)
{
	u32 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();

	u32 newCursor;
	bool success = math::AddInsideBounds(cursor, offset, GetSize(), newCursor);

	if(!success)
		throw io::FileException(io::FileException::OutsideFile);

	if(fseek(m_File, newCursor, 0) != 0)
		throw core::RuntimeException("fseek failed");
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

