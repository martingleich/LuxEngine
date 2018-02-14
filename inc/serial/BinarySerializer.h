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
		ObjectMap* objectMap = nullptr) :
		Serializer(inClass, outClass, objectMap),
		m_Link(outClass, inClass)
	{
		m_Block.sid = 0;
		m_LastAllocation.size = 0;

		// lxAssert(inClass == outClass);
	}

	void BeginStructure(u32 sid)
	{
		lxAssert(m_Block.sid == 0);

		auto& type = m_InClass->GetTypeInfo(sid);
		m_Block.sid = sid;
		m_Block.start = GetCursor();
		m_Block.end = m_Block.start + type.size;
	}

	void EndStructure()
	{
		Seek(m_Block.end, io::ESeekOrigin::Start);
		m_Block.sid = 0;
	}

	// Source: In-Class-Repr
	// Desination: File
	void WriteAll(const void* baseAddr)
	{
		auto sid = m_Block.sid;
		auto& type = m_InClass->GetTypeInfo(sid);
		lxAssert(type.isStructure);

		if(type.isCompact) {
			auto objectOffset = ((Structure&)type).elements[0].objectOffset;
			WriteBinary((u8*)baseAddr + objectOffset, type.size);
		} else {
			for(auto e : m_InClass->GetStructElements(sid)) {
				if(e.hasObjectData) {
					if(e.isArray)
						WriteArray(e.typeId, (u8*)baseAddr + e.objectOffset);
					else
						WriteBaseType(e.typeId, (u8*)baseAddr + e.objectOffset);
				} else {
					auto& type = m_InClass->GetTypeInfo(e.typeId);
					u32 null = 0;
					for(u32 i = 0; i < type.size; ++i)
						WriteBinary(&null, 1);
				}
			}
		}
	}

	virtual void WriteArray(u32 typeId, const void* data)
	{
		auto& array = *(const core::ArrayRawData*)data;
		size_t size = array.m_Used;
		size_t alloc = array.m_Alloc;
		void* ptr = array.m_Data;;

		u32 dword_size = (u32)size;
		u32 addr = AllocBehind(dword_size);
		WriteBinary(&dword_size, 4);
		WriteBinary(&addr, 4);

		auto old = GetCursor();
		Seek(addr, io::ESeekOrigin::Start);
		auto elemType = m_InClass->GetTypeInfo(typeId);
		for(u32 i = 0; i < dword_size; ++i)
			WriteBaseType(typeId, (u8*)ptr + i*elemType.size);
		Seek(old, io::ESeekOrigin::Start);
	}

	virtual void WriteElement(const char* element, const void* elemAddr)
	{
		auto elem = m_InClass->GetStructElement(m_Block.sid, element);
		u32 pos = m_Block.start + elem->offset;
		Seek(pos, io::ESeekOrigin::Start);

		if(elem->isArray)
			WriteArray(elem->typeId, elemAddr);
		else
			WriteBaseType(elem->typeId, elemAddr);
	}

	virtual void WriteBaseType(u32 typeId, const void* data)
	{
		auto type = m_InClass->GetTypeInfo(typeId);
		if(type.isCompact) {
			WriteBinary(data, type.size);
		} else {
			if(typeId == Type_String) {
				auto& str = *((core::String*)data);
				u32 size = str.Size();
				u32 addr = AllocBehind(size);
				WriteBinary(&size, 4);
				WriteBinary(&addr, 4);
				WriteBinary(addr, str.Data(), size);
			} else if(typeId == Type_StrongRef || typeId == Type_WeakRef) {
				WriteBinary(data, type.size);
			} else {
				lxAssertNeverReach("Unsupported type");
			}
		}
	}

	// Source: File
	// Desination: In-Class-Repr
	void ReadAll(void* baseAddr)
	{
		auto sid = m_Block.sid;
		auto type = m_InClass->GetTypeInfo(sid);
		lxAssert(type.isStructure);

		if(type.isCompact) {
			ReadBinary((u8*)baseAddr + ((Structure&)type).elements[0].objectOffset, type.size);
		} else {
			for(auto e : m_InClass->GetStructElements(sid)) {
				if(e.hasObjectData) {
					if(e.isArray)
						ReadArray(e.typeId, (u8*)baseAddr + e.objectOffset);
					else
						ReadBaseType(e.typeId, (u8*)baseAddr + e.objectOffset);
				} else {
					auto& type = m_InClass->GetTypeInfo(e.typeId);
					Seek(type.size);
				}
			}
		}
	}

	virtual void ReadArray(u32 typeId, void* data)
	{
		u32 dword_size, addr;
		ReadBinary(&dword_size, 4);
		ReadBinary(&addr, 4);

		auto& array = *(core::ArrayRawData*)data;
		size_t size = array.m_Used;
		size_t alloc = array.m_Alloc;
		void* ptr = array.m_Data;
		u32 inClassSize = GetCoreType(typeId).GetSize();
		if(alloc < dword_size) {
			array.ArrayFree(ptr);
			ptr = array.ArrayAllocate(dword_size * inClassSize);
			array.m_Used = dword_size;
			array.m_Alloc = dword_size;
			array.m_Data = ptr;
		}

		auto old = GetCursor();
		Seek(addr, io::ESeekOrigin::Start);
		for(u32 i = 0; i < dword_size; ++i)
			ReadBaseType(typeId, (u8*)ptr + i*inClassSize);
		Seek(old, io::ESeekOrigin::Start);
	}

	virtual void ReadElement(const char* element, void* elemAddr)
	{
		auto elem = m_InClass->GetStructElement(m_Block.sid, element);
		u32 pos = m_Block.start + elem->offset;
		Seek(pos, io::ESeekOrigin::Start);

		if(elem->isArray)
			ReadArray(elem->typeId, elemAddr);
		else
			ReadBaseType(elem->typeId, elemAddr);
	}

	virtual void ReadBaseType(u32 typeId, void* data)
	{
		auto type = m_InClass->GetTypeInfo(typeId);
		if(type.isCompact) {
			ReadBinary(data, type.size);
		} else {
			if(typeId == Type_String) {
				u32 size, addr;
				ReadBinary(&size, 4);
				ReadBinary(&addr, 4);
				m_Buffer.SetMinSize(size);
				ReadBinary(addr, m_Buffer, size);
				core::String& str = *((core::String*)data);
				str.Clear();
				str.AppendRaw(m_Buffer, size);
			} else if(typeId == Type_StrongRef || typeId == Type_WeakRef) {
				u32 inFileId;
				ReadBinary(&inFileId, type.size);
				auto objectId = m_ObjectMap->GetObject(inFileId);
				*((core::ID*)data) = objectId;
			} else {
				lxAssertNeverReach("Unsupported type");
			}
		}
	}

	virtual void WriteBinary(u32 pos, const void* data, u32 size) = 0;
	virtual void ReadBinary(u32 pos, void* data, u32 size) = 0;

	virtual void WriteBinary(const void* data, u32 size) = 0;
	virtual void ReadBinary(void* data, u32 size) = 0;

protected:
	u32 AllocBehind(u32 bytes)
	{
		auto cur = GetCursor();
		if(m_Block.end < cur)
			m_Block.end = cur;

		u32 out = m_Block.end;
		m_Block.end += bytes;
		m_LastAllocation.size = bytes;
		return out;
	}

	u32 ReallocBehind(u32 pos, u32 bytes)
	{
		auto lastAlloc = m_Block.end - m_LastAllocation.size;
		if(lastAlloc != pos)
			return AllocBehind(bytes);
		m_Block.end = lastAlloc + bytes;
		m_LastAllocation.size = bytes;
		return lastAlloc;
	}

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
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_BINARY_SERIALZER_H