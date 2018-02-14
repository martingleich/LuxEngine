#ifndef INCLUDED_SERIAL_STRUCTURAL_TABLE_H
#define INCLUDED_SERIAL_STRUCTURAL_TABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxHashMap.h"
#include "serial/SerialStructure.h"

namespace lux
{
namespace serial
{

class Serializable;

//! Contains a list of structures.
class StructuralTable : public ReferenceCounted
{
public:
	struct TypeCallable
	{
		const StructuralTable* table;
		TypeCallable() {}
		TypeCallable(const StructuralTable* _table) :
			table(_table)
		{}

		const Type& operator()(u32 i) const {
			return table->GetTypeInfo(i);
		}
	};
	struct StructureCallable
	{
		const StructuralTable* table;
		StructureCallable() {}
		StructureCallable(const StructuralTable* _table) :
			table(_table)
		{}

		const Structure& operator()(u32 i) const {
			return (const Structure&)table->GetTypeInfo(i);
		}
	};

	using ElementRange = core::Range<const StructureElement*>;
	using TypeIterator = core::IndexCallableIterator<Type, TypeCallable, u32>;
	using StructureIterator = core::IndexCallableIterator<Structure, StructureCallable, u32>;

public:
	//! The table of the currently running instance of the engine.
	LUX_API static StructuralTable* EngineTable();

	LUX_API StructuralTable();
	LUX_API ~StructuralTable();

	//! Add a new structure via builder.
	/**
	Usage:<br>
	auto builder = table->AddStructure("mytype", this);<br>
	builder.AddType("a_element", m_AMember);<br>
	builder.Finalize();<br>
	\param name The name of the structure.
	\param baseAddr A pointer to the type this structure is mapping.
	\param version The version of this structure.
	\return The structure builder.
	*/
	LUX_API StructureBuilder AddStructure(
		const core::String& name,
		const Serializable* baseAddr,
		u32 version = 0);

	//! Add a structure.
	/**
	\return The type id of the structure.
	*/
	LUX_API u32 AddStructure(const Structure& data);

	//! Get detailed typeinformation about a type id.
	/**
	If typeid does not exists an exception is thrown.
	*/
	LUX_API const Type& GetTypeInfo(u32 typeId) const;

	//! Get elements about a type id.
	/**
	If typeid does not exists an exception is thrown.
	If typeid isn't a structure an empty range is returned.
	*/
	LUX_API ElementRange GetStructElements(u32 typeId) const;

	//! Get the typeid based on the name of the type.
	LUX_API u32 GetTypeId(const core::String& str) const;

	//! Retrieve all types.
	LUX_API core::Range<TypeIterator> GetTypes() const;

	//! Retrieve all structures.
	LUX_API core::Range<StructureIterator> GetStructures() const;

	//! Retrieve a single element of a structure based on its name.
	/**
	If element doesn't exists null is returned.
	*/
	LUX_API const StructureElement* GetStructElement(u32 typeId, const core::String& name) const;

	//! Retrieve a single element of a structure based on its name.
	/**
	If element doesn't exists null is returned.
	*/
	LUX_API const StructureElement* GetStructElement(u32 typeId, const char* name) const;

	//! Retrieve a single element of a structure based on its id.
	LUX_API const StructureElement* GetStructElement(u32 typeId, u32 elemId) const;

private:
	u32 AddBaseType(const core::String& name, u32 size, bool complex = false);

private:
	using TypeMap = core::HashMap<core::SharedString, u32>;
	using ElemMap = core::HashMap<core::SharedString, u32>;

	core::Array<Type> m_BaseTypes;
	core::Array<Structure> m_StructureTypes;
	core::Array<ElemMap> m_ElementMaps;

	TypeMap m_TypeMap;
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_SERIAL_STRUCTURAL_TABLE_H