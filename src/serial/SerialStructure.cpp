#include "serial/SerialStructure.h"
#include "serial/StructuralTable.h"
#include "core/Referable.h"

namespace lux
{
namespace serial
{

core::Type GetCoreType(u32 baseType)
{
	switch(baseType) {
	case Type_Int: return core::Types::Integer();
	case Type_Float: return core::Types::Float();
	case Type_Vec2: return core::Types::Vector2f();
	case Type_Vec3: return core::Types::Vector3f();
	case Type_Colorf: return core::Types::Colorf();
	case Type_Color: return core::Types::Color();
	case Type_Quaternion: return core::Types::QuaternionF();
	case Type_Bool: return core::Types::Boolean();
	case Type_String: return core::Types::String();
	case Type_Blob: return core::Types::Unknown();
	case Type_StrongRef: return core::Types::StrongRef();
	case Type_WeakRef: return core::Types::WeakRef();
	default:
		lxAssertNeverReach("Unknown type");
		return core::Types::Unknown();
	}
}

u32 GetFromCoreType(core::Type type)
{
	if(type == core::Types::Integer()) return Type_Int;
	if(type == core::Types::Float()) return Type_Float;
	if(type == core::Types::Vector2f()) return Type_Vec2;
	if(type == core::Types::Vector3f()) return Type_Vec3;
	if(type == core::Types::Colorf()) return Type_Colorf;
	if(type == core::Types::Color()) return Type_Color;
	if(type == core::Types::QuaternionF()) return Type_Quaternion;
	if(type == core::Types::Boolean()) return Type_Bool;
	if(type == core::Types::String()) return Type_String;
	if(type == core::Types::StrongRef()) return Type_StrongRef;
	if(type == core::Types::WeakRef()) return Type_WeakRef;
	lxAssertNeverReach("Unknown type");
	return Type_Invalid;
}

StructureBuilder::StructureBuilder(
	StructuralTable* structTable,
	const void* base,
	const core::String& name,
	u32 version)
{
	m_StructTable = structTable;
	m_BasePtr = base;

	m_Data.name = core::SharedString(name);
	m_Data.size = 0;
	m_Data.isStructure = true;
	m_Data.isCompact = true;
	m_Data.version = version;

	m_NextOffset = 0;
}

u32 StructureBuilder::Finalize()
{
	return m_StructTable->AddStructure(m_Data);
}

void StructureBuilder::AddBaseType(u32 type, const core::String& name, const void* ptr)
{
	auto& typeInfo = m_StructTable->GetTypeInfo(type);
	AddType(type, typeInfo.size, false, name, ptr);
}

void StructureBuilder::AddArrayBaseType(u32 type, const core::String& name, const void* ptr)
{
	AddType(type, 4 + 4, true, name, ptr);
}

void StructureBuilder::AddType(
	u32 type,
	u32 size,
	bool array,
	const core::String& name,
	const void* ptr)
{
	auto& typeInfo = m_StructTable->GetTypeInfo(type);

	// Create the element
	StructureElement elem;
	elem.typeId = type;
	elem.offset = m_Data.size;
	elem.name = core::SharedString(name);
	elem.hasObjectData = (ptr != nullptr);
	elem.isArray = array;
	elem.objectOffset = elem.hasObjectData ? ((u8*)ptr - (u8*)m_BasePtr) : 0xFFFFFFFF;

	if(!elem.hasObjectData || elem.isArray || !typeInfo.isCompact || (m_NextOffset && elem.objectOffset != m_NextOffset))
		m_Data.isCompact = false;

	if(elem.hasObjectData)
		m_NextOffset = elem.objectOffset + size;

	m_Data.size += size;
	m_Data.elements.PushBack(elem);
}

} // namespace serial
} // namespace lux
