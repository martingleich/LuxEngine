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
		void* Buffer,
		const FileDescription& desc,
		String name,
		bool DeleteBufferOnDrop,
		bool Expandable,
		bool readOnly);
	~MemoryFile();
	u32 ReadBinary(u32 dwNumBytes, void* out);
	u32 WriteBinary(const void* data, u32 dwLength);
	bool Seek(s32 offset, ESeekOrigin Origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	u32 GetSize() const;
	u32 GetCursor() const;

private:
	u8* m_Buffer;
	u32 m_Size;
	u32 m_Cursor;
	bool m_IsEOF;

	const bool m_DeleteBufferOnDrop;
	const bool m_IsExpandable;
	const bool m_IsReadOnly;
};

} //namespace io
} //namespace lux

#endif
