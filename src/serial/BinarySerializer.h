#ifndef INCLUDED_SERIAL_BINARY_SERIALZER_H
#define INCLUDED_SERIAL_BINARY_SERIALZER_H
#include "serial/Serializer.h"

namespace lux
{
namespace serial
{

class BinarySerializer : public Serializer
{
public:
	BinarySerializer(const StructuralTable* inClass,
		const StructuralTable* outClass,
		ObjectMap* objectMap = nullptr);

	void BeginStructure(u32 sid);
	void EndStructure();

	void WriteAll(const void* baseAddr);
	void WriteArray(core::Type arr, core::Type base, const void* data);
	void WriteElement(const char* element, const void* elemAddr);
	void WriteType(core::Type type, const void* data);

	void ReadAll(void* baseAddr);
	void ReadArray(core::Type arr, core::Type base, void* data);
	void ReadElement(const char* element, void* elemAddr);
	void ReadType(core::Type type, void* data);

	virtual void WriteBinary(u32 pos, const void* data, u32 size) = 0;
	virtual void ReadBinary(u32 pos, void* data, u32 size) = 0;

	virtual void WriteBinary(const void* data, u32 size) = 0;
	virtual void ReadBinary(void* data, u32 size) = 0;

protected:
	u32 AllocBehind(u32 bytes);
	u32 ReallocBehind(u32 pos, u32 bytes);

protected:
	StructuralTableLink m_Link;
	core::RawMemory m_Buffer;

	struct Block
	{
		u32 sid;
		u32 start;
		u32 end;
	};
	Block m_Block;

	struct Allocation
	{
		u32 size;
	};

	Allocation m_LastAllocation;

	bool m_DoneAll;
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_BINARY_SERIALZER_H