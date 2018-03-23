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

s64 StreamFile::ReadBinaryPart(s64 numBytes, void* out)
{
	LX_CHECK_NULL_ARG(out);
	LX_CHECK_NULL_ARG(numBytes);
	if(numBytes > (s64)std::numeric_limits<size_t>::max())
		throw io::FileException(io::FileException::ReadError);

	if((s64)ftell(m_File) + numBytes > m_FileSize)
		numBytes = m_FileSize - ftell(m_File);

	s64 read = (s64)fread(out, (size_t)numBytes, 1, m_File)*numBytes;
	if(read != numBytes) {
		u8* cur = (u8*)out + read;
		s64 count = numBytes - read;
		while(count > 0) {
			s64 block = (s64)fread(cur, 1024, 1, m_File) * 1024;
			read += block;
			if(block != 1024)
				return read;

			cur += block;
			count -= block;
		}
	}

	return read;
}

s64 StreamFile::WriteBinaryPart(const void* data, s64 numBytes)
{
	LX_CHECK_NULL_ARG(data);
	LX_CHECK_NULL_ARG(numBytes);
	if(numBytes > (s64)std::numeric_limits<size_t>::max())
		throw io::FileException(io::FileException::WriteError);

	if((s64)ftell(m_File) > m_FileSize - numBytes)
		m_FileSize = ftell(m_File) + numBytes;

	if(fwrite(data, (size_t)numBytes, 1, m_File) != 1)
		throw io::FileException(io::FileException::WriteError);

	return numBytes;
}

void StreamFile::Seek(s64 offset, ESeekOrigin origin)
{
	s64 cursor = (origin == ESeekOrigin::Start) ? 0 : GetCursor();

	s64 newCursor = cursor + offset;
	if(newCursor < 0 || newCursor > GetSize())
		throw io::FileException(io::FileException::OutsideFile);

	if(fseek(m_File, (long)newCursor, 0) != 0)
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

s64 StreamFile::GetSize() const
{
	return m_FileSize;
}

s64 StreamFile::GetCursor() const
{
	return ftell(m_File);
}

}
}

