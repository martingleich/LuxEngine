#ifndef INCLUDED_LUX_MEMORY_FILE_H
#define INCLUDED_LUX_MEMORY_FILE_H
#include "io/File.h"

namespace lux
{
namespace io
{

class MemoryFile : public File
{
public:
	MemoryFile(
		void* buffer,
		const FileDescription& desc,
		const core::String& name,
		EVirtualCreateFlag flags);
	~MemoryFile();
	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;

private:
	u8* m_Buffer;
	size_t m_Size;
	size_t m_Cursor;
	bool m_IsEOF;

	EVirtualCreateFlag m_Flags;
};

} //namespace io
} //namespace lux

#endif
