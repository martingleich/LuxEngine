#include "serial/BinarySerializer.h"

namespace lux
{
namespace serial
{

BinarySerializer::BinarySerializer(const StructuralTable* inClass,
	const StructuralTable* outClass,
	ObjectMap* objectMap) :
	Serializer(inClass, outClass, objectMap),
	m_Link(outClass, inClass)
{
	m_Block.sid = 0;
	m_LastAllocation.size = 0;

	// TODO: Use m_Link to convert between inClass and outClass
	lxAssert(inClass == outClass);
}

void BinarySerializer::BeginStructure(u32 sid)
{
	lxAssert(m_Block.sid == 0);

	auto& type = m_InClass->GetStructure(sid);
	m_Block.sid = sid;
	m_Block.start = GetCursor();
	m_Block.end = m_Block.start + type.size;

	m_DoneAll = false;
}

void BinarySerializer::EndStructure()
{
	Seek(m_Block.end, io::ESeekOrigin::Start);
	m_Block.sid = 0;
}

// Source: In-Class-Repr
// Desination: File
void BinarySerializer::WriteAll(const void* baseAddr)
{
	if(m_DoneAll)
		return;
	auto& type = m_InClass->GetStructure(m_Block.sid);

	if(type.isCompact) {
		auto objectOffset = type.elements[0].objectOffset;
		WriteBinary((u8*)baseAddr + objectOffset, type.size);
	} else {
		for(auto& e : type.elements) {
			if(e.hasObjectData)
				WriteType(e.type, (u8*)baseAddr + e.objectOffset);
			else
				Seek(e.size);
		}
	}
	m_DoneAll = true;
}

void BinarySerializer::WriteArray(core::Type type, core::Type base, const void* data)
{
	core::VariableArrayAccess arrayAccess(data, type);
	int size = arrayAccess.Size();

	u32 dword_size = (u32)size;
	u32 addr = AllocBehind(dword_size);
	WriteBinary(&dword_size, 4);
	WriteBinary(&addr, 4);

	auto old = GetCursor();
	Seek(m_Block.start + addr, io::ESeekOrigin::Start);
	for(u32 i = 0; i < dword_size; ++i)
		WriteType(base, arrayAccess[i].Pointer());
	Seek(old, io::ESeekOrigin::Start);
}

void BinarySerializer::WriteElement(const char* element, const void* elemAddr)
{
	auto elem = m_InClass->GetStructureElement(m_Block.sid, element);
	u32 pos = m_Block.start + elem->offset;
	Seek(pos, io::ESeekOrigin::Start);

	WriteType(elem->type, elemAddr);
}

void BinarySerializer::WriteType(core::Type type, const void* data)
{
	if(type.IsTrivial()) {
		WriteBinary(data, type.GetSize());
	} else {
		if(type == core::Types::String()) {
			auto& str = *((core::String*)data);
			u32 size = str.Size();
			u32 addr = AllocBehind(size);
			WriteBinary(&size, 4);
			WriteBinary(&addr, 4);
			WriteBinary(m_Block.start + addr, str.Data(), size);
		} else if(core::Types::IsArray(type)) {
			WriteArray(type, core::Types::GetArrayBase(type), data);
		} else {
			lxAssertNeverReach("Unsupported type");
		}
	}
}

// Source: File
// Desination: In-Class-Repr
void BinarySerializer::ReadAll(void* baseAddr)
{
	if(m_DoneAll)
		return;

	auto& type = m_InClass->GetStructure(m_Block.sid);
	if(type.isCompact) {
		ReadBinary((u8*)baseAddr + type.elements[0].objectOffset, type.size);
	} else {
		for(auto& e : type.elements) {
			if(e.hasObjectData)
				ReadType(e.type, (u8*)baseAddr + e.objectOffset);
			else
				Seek(e.size);
		}
	}
	m_DoneAll = true;
}

void BinarySerializer::ReadArray(core::Type type, core::Type base, void* data)
{
	u32 dword_size, addr;
	ReadBinary(&dword_size, 4);
	ReadBinary(&addr, 4);

	core::VariableArrayAccess arrayAccess(data, type);

	arrayAccess.Resize(dword_size);

	auto old = GetCursor();
	Seek(m_Block.start + addr, io::ESeekOrigin::Start);
	for(u32 i = 0; i < dword_size; ++i)
		ReadType(base, arrayAccess[i].Pointer());
	Seek(old, io::ESeekOrigin::Start);
}

void BinarySerializer::ReadElement(const char* element, void* elemAddr)
{
	auto elem = m_InClass->GetStructureElement(m_Block.sid, element);
	u32 pos = m_Block.start + elem->offset;
	Seek(pos, io::ESeekOrigin::Start);

	ReadType(elem->type, elemAddr);
}

void BinarySerializer::ReadType(core::Type type, void* data)
{
	// IDs are complex to read.
	if(type.IsTrivial() && !core::Types::IsIDType(type)) {
		ReadBinary(data, type.GetSize());
	} else {
		if(type == core::Types::String()) {
			u32 size, addr;
			ReadBinary(&size, 4);
			ReadBinary(&addr, 4);
			m_Buffer.SetMinSize(size);
			ReadBinary(m_Block.start + addr, m_Buffer, size);
			core::String& str = *((core::String*)data);
			str.Clear();
			str.AppendRaw(m_Buffer, size);
		} else if(core::Types::IsIDType(type)) {
			u32 inFileId;
			ReadBinary(&inFileId, type.GetSize());
			auto objectId = m_ObjectMap->GetObject(inFileId);
			*((core::ID*)data) = objectId;
		} else if(core::Types::IsArray(type)) {
			ReadArray(type, core::Types::GetArrayBase(type), data);
		} else {
			lxAssertNeverReach("Unsupported type");
		}
	}
}

u32 BinarySerializer::AllocBehind(u32 bytes)
{
	auto cur = GetCursor();
	if(m_Block.end < cur)
		m_Block.end = cur;

	u32 out = m_Block.end - m_Block.start;
	m_Block.end += bytes;
	m_LastAllocation.size = bytes;
	return out;
}

u32 BinarySerializer::ReallocBehind(u32 pos, u32 bytes)
{
	auto absLastAlloc = m_Block.end - m_LastAllocation.size;
	if(absLastAlloc != pos)
		return AllocBehind(bytes);
	m_Block.end = absLastAlloc + bytes;
	m_LastAllocation.size = bytes;
	return absLastAlloc - m_Block.start;
}

} // namespace serial
} // namespace lux
