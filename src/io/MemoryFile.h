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
		const FileInfo& desc,
		const Path& path,
		EVirtualCreateFlag flags);
	MemoryFile(
		const void* buffer,
		const FileInfo& desc,
		const Path& path,
		EVirtualCreateFlag flags) :
		MemoryFile(const_cast<void*>(buffer),
			desc,
			path,
			flags | EVirtualCreateFlag::ReadOnly)
	{}
	~MemoryFile();
	s64 ReadBinaryPart(s64 numBytes, void* out);
	s64 WriteBinaryPart(const void* data, s64 length);
	void Seek(s64 offset, ESeekOrigin origin = ESeekOrigin::Cursor);
	void* GetBuffer();
	const void* GetBuffer() const;
	s64 GetSize() const;
	s64 GetCursor() const;
	const FileInfo& GetInfo() const { return m_Info; }
	const Path& GetPath() const { return m_Path; }

private:
	u8* m_Buffer;
	FileInfo m_Info;
	size_t m_Size;
	Path m_Path;
	size_t m_Cursor;
	bool m_IsEOF;

	EVirtualCreateFlag m_Flags;
};

} //namespace io
} //namespace lux

#endif
