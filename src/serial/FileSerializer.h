#ifndef INCLUDED_SERIAL_FILE_SERIALIZER_H
#define INCLUDED_SERIAL_FILE_SERIALIZER_H
#include "serial/BinarySerializer.h"
#include "io/File.h"

namespace lux
{
namespace serial
{

class FileSerializer : public BinarySerializer
{
public:
	FileSerializer(io::File* file, const StructuralTable* table) :
		BinarySerializer(table, table),
		m_File(file)
	{
	}


	void WriteBinary(u32 pos, const void* data, u32 size)
	{
		m_File->Seek(pos, io::ESeekOrigin::Start);
		m_File->WriteBinary(data, size);
	}
	void ReadBinary(u32 pos, void* data, u32 size)
	{
		m_File->Seek(pos, io::ESeekOrigin::Start);
		m_File->ReadBinary(size, data);
	}
	void WriteBinary(const void* data, u32 size)
	{
		m_File->WriteBinary(data, size);
	}
	void ReadBinary(void* data, u32 size)
	{
		m_File->ReadBinary(size, data);
	}

	void Seek(u32 move, io::ESeekOrigin origin = io::ESeekOrigin::Cursor)
	{
		m_File->Seek(move, origin);
	}
	
	u32 GetCursor() const
	{
		return m_File->GetCursor();
	}

private:
	StrongRef<io::File> m_File;
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_MEMORY_SERIALIZER_H
