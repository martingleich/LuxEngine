#include "serial/SerialStructure.h"
#include "serial/StructuralTable.h"
#include "core/Referable.h"

namespace lux
{
namespace serial
{

StructureBuilder::StructureBuilder(
	StructuralTable* structTable,
	const void* base,
	const core::String& name,
	u32 version)
{
	m_StructTable = structTable;
	m_BasePtr = base;

	m_Structure.name = core::SharedString(name);
	m_Structure.size = 0;
	m_Structure.isCompact = true;
	m_Structure.version = version;

	m_NextOffset = 0;
}

u32 StructureBuilder::Finalize()
{
	if(m_Structure.elements.IsEmpty())
		return 0;
	return m_StructTable->AddStructure(m_Structure);
}

void StructureBuilder::AddElement(
	core::Type type,
	const core::String& name,
	const void* ptr)
{
	StructureElement elem;
	elem.name = core::SharedString(name);
	elem.type = type;
	elem.elemId = m_NextElemId;
	elem.offset = m_Structure.size;
	elem.objectOffset = (ptr != nullptr) ? static_cast<u32>((u8*)ptr - (u8*)m_BasePtr) : 0xFFFFFFFF;
	elem.hasObjectData = (ptr != nullptr);
	if(!type.IsTrivial()) {
		if(type == core::Types::String())
			elem.size = 4 + 4;
		else if(core::Types::IsArray(type))
			elem.size = 4 + 4;
		else
			throw core::NotImplementedException();
	}
	m_Structure.elements.PushBack(elem);
	m_Structure.size += type.GetSize();

	if(!elem.hasObjectData || type.IsTrivial() || (m_NextOffset && elem.objectOffset != m_NextOffset))
		m_Structure.isCompact = false;

	if(elem.hasObjectData)
		m_NextOffset = elem.objectOffset + elem.size;
	++m_NextElemId;
}

} // namespace serial
} // namespace lux

#include "serial/BinarySerializer.h"
