#ifndef INCLUDED_REFERABLEFACTORY_H
#define INCLUDED_REFERABLEFACTORY_H
#include "core/Referable.h"

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

	//! Access the global referable factory
	LUX_API static ReferableFactory* Instance();

	//! Destroys the global referable factory
	LUX_API static void Destroy();

	//! Register a new type
	/**
	The type name, resource type, type id are automatic read from the object.
	\param prototype The new prototype object for this type, every new object is cloned from this one
	*/
	virtual void RegisterType(Referable* prototype) = 0;

	//! Unregister a type
	/**
	\param type The main type
	\param subType The sub type
	*/
	virtual void UnregisterType(Name type, Name subType) = 0;

	//! Set the passed referable as prototype for its own type.
	/**
	\param prototype The new prototype type.
	*/
	virtual void SetPrototype(Referable* prototype) = 0;

	//! Gets the prototype object
	/**
	\param type The type
	\param subType The sub type
	\return The prototype object
	\throws ObjectNotFoundException No Prototye with passed name exist.
	*/
	virtual StrongRef<Referable> GetPrototype(Name type, Name subType) const = 0;

	//! Gets the prototype object
	/**
	\param id The internal id of the object.
	\return The prototype object
	\throws OutOfRange id is out of range
	*/
	virtual StrongRef<Referable> GetPrototype(size_t id) const = 0;

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
