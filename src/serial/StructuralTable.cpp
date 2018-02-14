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
	lxAssert(AddBaseType("int", 4, false) == Type_Int);
	lxAssert(AddBaseType("float", 4, false) == Type_Float);
	lxAssert(AddBaseType("vec2", 2 * 4, false) == Type_Vec2);
	lxAssert(AddBaseType("vec3", 3 * 4, false) == Type_Vec3);
	lxAssert(AddBaseType("colorf", 4 * 4, false) == Type_Colorf);
	lxAssert(AddBaseType("color", 4, false) == Type_Color);
	lxAssert(AddBaseType("quaternion", 4 * 4, false) == Type_Quaternion);
	lxAssert(AddBaseType("bool", 1, false) == Type_Bool);

	lxAssert(AddBaseType("string", 4 + 4, true) == Type_String);
	lxAssert(AddBaseType("blob", 4 + 4, true) == Type_Blob);
	lxAssert(AddBaseType("strongref", 4, true) == Type_StrongRef);
	lxAssert(AddBaseType("weakref", 4, true) == Type_WeakRef);
}

StructuralTable::~StructuralTable()
{
}

StructureBuilder StructuralTable::AddStructure(const core::String& name,
	const Serializable* baseAddr, u32 version)
{
	return StructureBuilder(this, baseAddr, name, version);
}

u32 StructuralTable::AddStructure(const Structure& data)
{
	m_StructureTypes.PushBack(data);
	auto& newStructure = m_StructureTypes.Back();
	newStructure.typeId = m_BaseTypes.Size() + m_StructureTypes.Size();

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

const Type& StructuralTable::GetTypeInfo(u32 typeId) const
{
	lxAssert(typeId != 0);
	if(typeId == 0)
		throw core::OutOfRangeException();

	--typeId;
	if(typeId < m_BaseTypes.Size())
		return m_BaseTypes.At(typeId);
	else
		return m_StructureTypes.At(typeId - m_BaseTypes.Size());
}

StructuralTable::ElementRange StructuralTable::GetStructElements(u32 typeId) const
{
	auto& typeInfo = GetTypeInfo(typeId);
	if(!typeInfo.isStructure)
		return ElementRange(nullptr, nullptr);

	auto& structure = (Structure&)typeInfo;
	return ElementRange(
		structure.elements.Data_c(),
		structure.elements.Data_c() + structure.elements.Size());
}

core::Range<StructuralTable::TypeIterator> StructuralTable::GetTypes() const
{
	TypeCallable call(this);
	return core::MakeRange(
		TypeIterator(1, call),
		TypeIterator(1 + m_BaseTypes.Size() + m_StructureTypes.Size(), call));
}

core::Range<StructuralTable::StructureIterator> StructuralTable::GetStructures() const
{
	StructureCallable call(this);
	return core::MakeRange(
		StructureIterator(1 + m_BaseTypes.Size(), call),
		StructureIterator(1 + m_BaseTypes.Size() + m_StructureTypes.Size(), call));
}

u32 StructuralTable::GetTypeId(const core::String& name) const
{
	return m_TypeMap.At<core::String>(name, 0);
}

const StructureElement* StructuralTable::GetStructElement(u32 typeId, const core::String& name) const
{
	lxAssert(typeId != 0);

	auto elemMap = m_ElementMaps.At(typeId - 1);
	auto elemId = elemMap.Find(name);
	if(elemId == elemMap.End())
		return nullptr;
	return GetStructElement(typeId, *elemId);
}

const StructureElement* StructuralTable::GetStructElement(u32 typeId, const char* name) const
{
	lxAssert(typeId != 0);

	auto elemMap = m_ElementMaps.At(typeId - 1);
	auto elemId = elemMap.Find(name);
	if(elemId == elemMap.End())
		return nullptr;
	return GetStructElement(typeId, *elemId);
}

const StructureElement* StructuralTable::GetStructElement(u32 typeId, u32 elemId) const
{
	lxAssert(typeId != 0);

	return GetStructElements(typeId).begin() + elemId;
}

u32 StructuralTable::AddBaseType(const core::String& name, u32 size, bool complex)
{
	Type type;
	type.typeId = m_BaseTypes.Size() + 1;
	type.version = 0;
	type.name = core::SharedString(name);
	type.size = size;
	type.isCompact = !complex;
	type.isStructure = false;

	m_BaseTypes.PushBack(type);
	m_TypeMap.Set(type.name, type.typeId);
	m_ElementMaps.EmplaceBack();

	return type.typeId;
}

} // namespace serial
} // namespace lux
