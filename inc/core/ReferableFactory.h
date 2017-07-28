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
	typedef Referable* (*CreationFunc)(const void*);

public:
	virtual ~ReferableFactory()
	{
	}

	//! Initialize the global referable factory
	LUX_API static void Initialize(ReferableFactory* refFactory=nullptr);

	//! Access the global referable factory
	LUX_API static ReferableFactory* Instance();

	//! Destroys the global referable factory
	LUX_API static void Destroy();

	//! Register a new type
	/**
	The type name, resource type, type id are automatic read from the object.
	\param prototype The new prototype object for this type, every new object is cloned from this one
	*/
	virtual void RegisterType(Name name, CreationFunc create) = 0;

	//! Unregister a type
	/**
	\param type The main type
	\param subType The sub type
	*/
	virtual void UnregisterType(Name type) = 0;

	//! Create a new object
	/**
	\param type The type
	\param data Additinal data to pass to constructor, depends on type.
	\return The new object
	*/
	virtual StrongRef<Referable> Create(Name type, const void* data=nullptr) = 0;

	//! The total number of types
	/**
	\return The total number of types
	*/
	virtual size_t GetTypeCount() const = 0;

	//! Get a new id
	virtual lxID MakeId(Referable* r) = 0; 

	//! Mark a id as unused
	virtual void FreeId(lxID id) = 0; 
};

}
}

#endif // !INCLUDED_IREFERABLEFACTORY_H
