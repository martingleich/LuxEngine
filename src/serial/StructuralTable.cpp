#include "serial/StructuralTable.h"

namespace lux
{
namespace serial
{
static StrongRef<StructuralTable> g_EngineTable;

StructuralTable* StructuralTable::EngineTable()
{
	if(!g_EngineTable)
		g_EngineTable = LUX_NEW(StructuralTable);
	return g_EngineTable;
}

StructuralTable::StructuralTable()
{
}

StructuralTable::StructuralTable(const StructuralTable&)
{
}

StructuralTable::~StructuralTable()
{
}

StructureBuilder StructuralTable::AddStructure(
	core::StringView name, const Serializable* baseAddr, u32 version)
{
	return StructureBuilder(this, baseAddr, name, version);
}

u32 StructuralTable::AddStructure(const Structure& data)
{
	if(data.elements.IsEmpty())
		return 0;
	m_StructureTypes.PushBack(data);

	auto& newStructure = m_StructureTypes.Back();
	newStructure.typeId = m_StructureTypes.Size();

	m_TypeMap.Set(data.name, newStructure.typeId);

	auto& elemMap = m_ElementMaps.EmplaceBack();
	elemMap.Reserve(newStructure.elements.Size());
	u32 elemId = 0;
	for(auto& e : newStructure.elements) {
		elemMap.Set(e.name, elemId);
		++elemId;
	}

	return newStructure.typeId;
}

const Structure& StructuralTable::GetStructure(u32 sid) const
{
	lxAssert(sid != 0);
	return m_StructureTypes.At(sid - 1);
}

core::Range<StructuralTable::StructureIterator> StructuralTable::GetStructures() const
{
	StructureCallable call(this);
	return core::MakeRange(
		StructureIterator(1, call),
		StructureIterator(1 + m_StructureTypes.Size(), call));
}

u32 StructuralTable::GetStructureId(const core::String& name) const
{
	return m_TypeMap.Get(name, 0);
}

const StructureElement* StructuralTable::GetStructureElement(u32 sid, const core::String& name) const
{
	lxAssert(sid != 0);

	auto elemMap = m_ElementMaps.At(sid - 1);
	auto elemId = elemMap.Find(name);
	if(elemId == elemMap.end())
		return nullptr;
	return GetStructureElement(sid, elemId->value);
}

const StructureElement* StructuralTable::GetStructureElement(u32 sid, const char* name) const
{
	lxAssert(sid != 0);

	auto elemMap = m_ElementMaps.At(sid - 1);
	auto elemId = elemMap.Find(name);
	if(elemId == elemMap.end())
		return nullptr;
	return GetStructureElement(sid, elemId->value);
}

const StructureElement* StructuralTable::GetStructureElement(u32 sid, u32 elemId) const
{
	lxAssert(sid != 0);

	return &GetStructure(sid).elements.At(elemId);
}

} // namespace serial
} // namespace lux
