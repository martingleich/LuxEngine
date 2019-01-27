#ifndef INCLUDED_LUX_REFERABLE_H
#define INCLUDED_LUX_REFERABLE_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "core/lxTypes.h"
#include "core/lxID.h"
#include "core/VariableAccess.h"
#include "serial/Serializer.h"

namespace lux
{
namespace core
{
//! A referable object
/**
Referable objects can be cloned from older instances.
They also can be used with the \ref ReferableFactory and created there my name.
*/
class Referable : public serial::Serializable
{
public:
	Referable() :
		m_Id(core::IDManager::Instance()->Create(this))
	{
	}
	Referable(const Referable&) :
		m_Id(core::IDManager::Instance()->Create(this))
	{
	}
	virtual ~Referable()
	{
		core::IDManager::Instance()->Release(m_Id);
	}
	Referable& operator=(const Referable& other) = delete;

	//! Serialize object into target.
	virtual void Serialize(serial::Serializer* target)
	{
		target->WriteAll(static_cast<serial::Serializable*>(this));
	}

	//! Deserialize object from source.
	virtual void DeSerialize(serial::Serializer* source)
	{
		source->ReadAll(static_cast<serial::Serializable*>(this));
	}

	//! Retrieve value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\param var The attribute is copied into the VariableAccess, must be of the same type as the attribute.
	*/
	LUX_API virtual void GetAttribute(const core::String& name, core::VariableAccess var);

	//! Retrieve value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\return A variableAccess to the attribute.
	*/
	LUX_API virtual core::VariableAccess GetAttribute(const core::String& name);

	//! Set value of a single attribute via name.
	/**
	\param name The name of the attribute.
	\param var The attribute is from this VariableAccess, must be of the same type as the attribute.
	*/
	LUX_API virtual void SetAttribute(const core::String& name, core::VariableAccess var);

	//! Called before any other serialization function.
	/*
	Must be implemented by the user. Should add elements to the builder. Must not
	call finalize.<br>
	*/
	virtual void InitSerializer(serial::StructureBuilder& builder) const
	{
		LUX_UNUSED(builder);
	}

	//! Return the type id of the mapped structure.
	/**
	Automatically implemented by LX_SERIAL_MEMBERS.
	*/
	StrongRef<core::Referable> Clone() const
	{
		return CloneImpl();
	}

	//! Get the id of the serializer data for this object.
	virtual u32 GetSerializerStructure() const { return 0; }

	//! Get the name of the referable type
	/**
	Must be unique over all types
	\return The name of the referable type
	*/
	virtual core::Name GetReferableType() const = 0;

	core::ID GetUniqueId()
	{
		return m_Id;
	}

protected:
	virtual StrongRef<core::Referable> CloneImpl() const
	{
		throw core::NotImplementedException("Referable::CloneImpl");
	}

private:
	core::ID m_Id;
};

namespace impl_referableRegister
{
struct ReferableRegisterBlock;

// Implemented in ReferableFactoryImpl.cpp
LUX_API void RegisterReferableBlock(ReferableRegisterBlock* block);
LUX_API void RunAllRegisterReferableFunctions();

struct ReferableRegisterBlock
{
	core::Name type;
	core::Referable* (*creator)(const void*);
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(core::Name t, core::Referable* (*_creator)(const void*)) :
		type(t),
		creator(_creator),
		nextBlock(nullptr)
	{
		RegisterReferableBlock(this);
	}
};
} // namespace impl_ReferableRegister

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the class definition in the header.
At best directly at the begin of the class.
*/
#define LX_REFERABLE_MEMBERS(class) \
public: \
::lux::core::Name GetReferableType() const; \
::lux::StrongRef<class> Clone() const; \
::lux::u32 GetSerializerStructure() const; \
protected: \
::lux::StrongRef<::lux::core::Referable> CloneImpl() const; \
private: \
static ::lux::u32 SERIAL_TYPE_ID; \
static ::lux::core::Name REFERABLE_TYPE_NAME;

//! Declare default referable members
/**
Use inside class inherited from Referable.
\param API API used to declare members.
*/
#define LX_REFERABLE_MEMBERS_API(class, API) \
public: \
API ::lux::core::Name GetReferableType() const; \
API ::lux::StrongRef<class> Clone() const; \
API ::lux::u32 GetSerializerStructure() const; \
protected: \
API ::lux::StrongRef<::lux::core::Referable> CloneImpl() const; \
private: \
static ::lux::u32 SERIAL_TYPE_ID; \
static ::lux::core::Name REFERABLE_TYPE_NAME;

//! Register referable class with ReferableFactory
/**
Use in global namespace in a source file.
\param ref_name C-String or core::Name containing the name of the class
\param class Fully classified name of the class
*/
#define LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
static ::lux::core::Referable* LX_CONCAT(InternalCreatorFunc_, __LINE__)(const void*) { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock LX_CONCAT(InternalReferableRegisterStaticObject_, __LINE__)(::lux::core::Name(ref_name), &LX_CONCAT(InternalCreatorFunc_, __LINE__));

#define LX_REFERABLE_MEMBERS_SRC_SERIAL(class, ref_name) \
::lux::u32 class::SERIAL_TYPE_ID = 0; \
::lux::u32 class::GetSerializerStructure() const { \
	if(SERIAL_TYPE_ID == (u32)-1) \
		return 0; \
	if(SERIAL_TYPE_ID == 0) { \
		auto builder = ::lux::serial::StructuralTable::EngineTable()->AddStructure(GetReferableType().AsView(), this); \
		InitSerializer(builder); \
		SERIAL_TYPE_ID = builder.Finalize(); \
		if(SERIAL_TYPE_ID == 0) \
			SERIAL_TYPE_ID = (::lux::u32)-1; \
	} \
	return SERIAL_TYPE_ID; \
}

//! Helper macro to declare all members for Referable classes
/**
Must be placed in the global namespace in the source file.
\param class The fully qualified name of the class.
\param ref_name The refereable type name for this class.
*/
#define LX_REFERABLE_MEMBERS_SRC(class, ref_name) \
LX_REGISTER_REFERABLE_CLASS(class, ref_name) \
::lux::core::Name class::REFERABLE_TYPE_NAME = ::lux::core::Name(ref_name); \
::lux::core::Name class::GetReferableType() const { return REFERABLE_TYPE_NAME;} \
::lux::StrongRef<::lux::core::Referable> class::CloneImpl() const { return LUX_NEW(class)(*this); } \
::lux::StrongRef<class> class::Clone() const { return CloneImpl().StaticCastStrong<class>(); } \
LX_REFERABLE_MEMBERS_SRC_SERIAL(class, ref_name)

} // namespace core
} // namespace lux

#endif // INCLUDED_LUX_REFERABLE_H
