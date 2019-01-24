#include "core/Referable.h"

namespace lux
{
namespace core
{
void Referable::GetAttribute(const core::String& name, core::VariableAccess var)
{
	auto elem = serial::StructuralTable::EngineTable()->GetStructureElement(GetSerializerStructure(), name);
	if(elem)
		var.AssignData((u8*)((Serializable*)this) + elem->objectOffset);
}

//! Retrieve value of a single attribute via name.
/**
\param name The name of the attribute.
\return A variableAccess to the attribute.
*/
 core::VariableAccess Referable::GetAttribute(const core::String& name)
{
	auto elem = serial::StructuralTable::EngineTable()->GetStructureElement(GetSerializerStructure(), name);
	if(elem)
		return core::VariableAccess(elem->type.GetConstantType(), (u8*)((Serializable*)this) + elem->objectOffset);
	else
		return core::VariableAccess();
}

//! Set value of a single attribute via name.
/**
\param name The name of the attribute.
\param var The attribute is from this VariableAccess, must be of the same type as the attribute.
*/
 void Referable::SetAttribute(const core::String& name, core::VariableAccess var)
{
	auto elem = serial::StructuralTable::EngineTable()->GetStructureElement(GetSerializerStructure(), name);
	if(elem)
		var.CopyData((u8*)((Serializable*)this) + elem->objectOffset);
}

} // namespace core
} // namespace lux