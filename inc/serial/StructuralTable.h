#ifndef INCLUDED_LUX_SERIAL_STRUCTURAL_TABLE_H
#define INCLUDED_LUX_SERIAL_STRUCTURAL_TABLE_H
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
	struct StructureCallable
	{
		const StructuralTable* table;
		StructureCallable() {}
		StructureCallable(const StructuralTable* _table) :
			table(_table)
		{
		}

		const Structure& operator()(u32 i) const
		{
			return table->GetStructure(i);
		}
	};

	using ElementRange = core::Range<const StructureElement*>;
	using StructureIterator = core::IndexCallableIterator<Structure, StructureCallable, u32>;

public:
	//! The table of the currently running instance of the engine.
	LUX_API static StructuralTable* EngineTable();

	LUX_API StructuralTable();
	LUX_API StructuralTable(const StructuralTable& other);
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

	//! Get structure information based on its id.
	/**
	If id does not exists an exception is thrown.
	*/
	LUX_API const Structure& GetStructure(u32 sid) const;

	//! Get the id based on the name of the structure.
	LUX_API u32 GetStructureId(const core::String& str) const;

	//! Retrieve all structures.
	LUX_API core::Range<StructureIterator> GetStructures() const;

	//! Retrieve a single element of a structure based on its name.
	/**
	If element doesn't exists null is returned.
	*/
	LUX_API const StructureElement* GetStructureElement(u32 sid, const core::String& name) const;

	//! Retrieve a single element of a structure based on its name.
	/**
	If element doesn't exists null is returned.
	*/
	LUX_API const StructureElement* GetStructureElement(u32 sid, const char* name) const;

	//! Retrieve a single element of a structure based on its id.
	LUX_API const StructureElement* GetStructureElement(u32 sid, u32 elemId) const;

private:
	using TypeMap = core::HashMap<core::SharedString, u32>;
	using ElemMap = core::HashMap<core::SharedString, u32>;

	core::Array<Structure> m_StructureTypes;
	core::Array<ElemMap> m_ElementMaps;

	TypeMap m_TypeMap;
};

} // namespace serial
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SERIAL_STRUCTURAL_TABLE_H