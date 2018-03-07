#ifndef INCLUDED_SERIAL_MEMORY_SERIALIZER_H
#define INCLUDED_SERIAL_MEMORY_SERIALIZER_H
#include "serial/BinarySerializer.h"

namespace lux
{
namespace serial
{

class MemorySerializer : public BinarySerializer
{
public:
	MemorySerializer(core::RawMemory& mem,
		const StructuralTable* table, ObjectMap* map = nullptr) :
		BinarySerializer(table, table, map),
		m_Memory(mem),
		m_Cursor(0)
	{
	}

	void WriteBinary(u32 pos, const void* data, u32 size)
	{
		m_Memory.SetMinSize(pos + size, core::RawMemory::COPY);
		memcpy((u8*)m_Memory + pos, data, size);
	}

	void ReadBinary(u32 pos, void* data, u32 size)
	{
		memcpy(data, (u8*)m_Memory + pos, size);
	}

	void WriteBinary(const void* data, u32 size)
	{
		WriteBinary(m_Cursor, data, size);
		m_Cursor += size;
	}

	void ReadBinary(void* data, u32 size)
	{
		ReadBinary(m_Cursor, data, size);
		m_Cursor += size;
	}

	void Seek(u32 move, io::ESeekOrigin origin = io::ESeekOrigin::Cursor)
	{
		if(origin == io::ESeekOrigin::Cursor)
			m_Cursor += move;
		else
			m_Cursor = move;
	}

	u32 GetCursor() const
	{
		return m_Cursor;
	}

private:
	core::RawMemory& m_Memory;
	u32 m_Cursor;
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_MEMORY_SERIALIZER_H