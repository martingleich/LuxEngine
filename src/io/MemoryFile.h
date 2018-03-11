#ifndef INCLUDED_MEMORY_FILE_H
#define INCLUDED_MEMORY_FILE_H
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
	u32 ReadBinaryPart(u32 numBytes, void* out);
	u32 WriteBinaryPart(const void* data, u32 length);
	void Seek(u32 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	u32 GetSize() const;
	u32 GetCursor() const;

private:
	u8* m_Buffer;
	u32 m_Size;
	u32 m_Cursor;
	bool m_IsEOF;

	EVirtualCreateFlag m_Flags;
};

} //namespace io
} //namespace lux

#endif
