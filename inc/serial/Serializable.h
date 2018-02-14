#ifndef INCLUDED_SERIAL_SERIALIZABLE_H
#define INCLUDED_SERIAL_SERIALIZABLE_H
#include "serial/Serializer.h"

namespace lux
{
namespace serial
{

//! Base class for serializable classes.
class Serializable : public Referable
{
public:
	Serializable()
	{
		u32& typeId = GetSerializerStructure();
		if(!typeId) {
			auto builder = serial::StructuralTable::EngineTable()->AddStructure(GetReferableType().c_str(), this);
			InitSerializer(builder);
			typeId = builder.Finalize();
			if(typeId == 0)
				throw core::RuntimeException("Can't run InitSerializer for Serizable object");
		}
	}

	//! Serialize object into target.
	virtual void Serialize(Serializer* target)
	{
		target->WriteAll((Serializable*)this);
	}
	
	//! Deserialize object from source.
	virtual void DeSerialize(Serializer* source)
	{
		source->ReadAll((Serializable*)this);
	}

	//! Retrieve value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\param var The attribute is copied into the VariableAccess, must be of the same type as the attribute.
	*/
	virtual void GetAttribute(const core::String& name, core::VariableAccess var)
	{
		auto elem = StructuralTable::EngineTable()->GetStructElement(GetSerializerStructure(), name);
		if(elem)
			var.AssignData((u8*)((Serializable*)this) + elem->objectOffset);
	}
	//! Retrieve value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\return A variableAccess to the attribute.
	*/
	virtual core::VariableAccess GetAttribute(const core::String& name)
	{
		auto elem = StructuralTable::EngineTable()->GetStructElement(GetSerializerStructure(), name);
		if(elem)
			return core::VariableAccess(serial::GetCoreType(elem->typeId).GetConstantType(), (u8*)((Serializable*)this) + elem->objectOffset);
		else
			return core::VariableAccess();
	}

	//! Set value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\param var The attribute is from this VariableAccess, must be of the same type as the attribute.
	*/
	virtual void SetAttribute(const core::String& name, core::VariableAccess var)
	{
		auto elem = StructuralTable::EngineTable()->GetStructElement(GetSerializerStructure(), name);
		if(elem)
			var.CopyData((u8*)((Serializable*)this) + elem->objectOffset);
	}

	// Data used for serialization

	//! Called before any other serialization function.
	/*
	Must be implemented by the user. Should add elements to the builder. Must not
	call finalize.<br>
	*/
	virtual void InitSerializer(serial::StructureBuilder& builder) const = 0;

	//! Return the type id of the mapped structure.
	/**
	Automatically implemented by LX_SERIAL_MEMBERS.
	Must be initialized to zero.
	*/
	virtual u32& GetSerializerStructure() const = 0;
};

} // namespace serial
} // namespace lux

//! Helper macro to declare all members for Serializable classes
/**
Also declares referable members. Must be placed in the class definition in the header.
*/
#define LX_SERIAL_MEMBERS() \
LX_REFERABLE_MEMBERS() \
private: static ::lux::u32 TYPE_ID; \
public: ::lux::u32& GetSerializerStructure() const { return TYPE_ID; }

//! Helper macro to declare all members for Serializable classes
/**
Also declares referable members.
Must be placed in the global namespace in the source file.
\param class The fully qualified name of the class.
\param ref_name The refereable type name for this class.
*/
#define LX_SERIAL_MEMBERS_SRC(class, ref_name) \
LX_REFERABLE_MEMBERS_SRC(class, ref_name) \
::lux::u32 class::TYPE_ID = 0;

#endif // #ifndef INCLUDED_SERIAL_SERIALIZABLE_H