#ifndef INCLUDED_REFERABLEFACTORY_H
#define INCLUDED_REFERABLEFACTORY_H
#include "resources/Referable.h"

namespace lux
{
namespace core
{

class ReferableFactory : public ReferenceCounted
{
public:
	//! The TypeID for any object in the ReferableFactory
	/**
	To retrieve this id use \ref GetTypeID
	*/
	typedef u32 TypeID;

public:
	virtual ~ReferableFactory()
	{
	}

	//! Register a new type
	/**
	The type name, resource type, type id are automatic read from the object.
	\param default The new default object for this type, every new object is cloned from this one
	\return True if the type was registed otherwise false.
	*/
	virtual bool RegisterType(Referable* default) = 0;

	//! Unregister a type
	/**
	\param type The main type
	\param subType The sub type
	*/
	virtual void UnregisterType(Name type, Name subType) = 0;

	//! Set the passed referable as default for its own type.
	/**
	\param default The new default type.
	\return Was the new default set
	*/
	virtual bool SetDefault(Referable* default) = 0;

	//! Gets the default object
	/**
	\param type The type
	\param subType The sub type
	\return The default object
	*/
	virtual StrongRef<Referable> GetDefault(Name type, Name subType) const = 0;

	//! Gets the default object
	/**
	\param id The internal id of the object.
	\return The default object
	*/
	virtual StrongRef<Referable> GetDefault(size_t id) const = 0;

	//! Create a new object
	/**
	A new id is assigned automatically.
	\param type The type
	\param subType The sub type 
	\return The new object
	*/
	virtual StrongRef<Referable> Create(Name type, Name subType) = 0;

	//! Create a new object
	/**
	\param type The type
	\param subType The sub type 
	\param id The id of the new object
	\return The new object
	*/
	virtual StrongRef<Referable> Create(Name type, Name subType, lxID id) = 0;

	//! The total number of types
	/**
	\return The total number of types
	*/
	virtual size_t GetTypeCount() const = 0;

	//! Creates a new unused id.
	virtual lxID MakeId() = 0; 
};

}
}

#endif // !INCLUDED_IREFERABLEFACTORY_H
