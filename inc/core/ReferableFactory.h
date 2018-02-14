#ifndef INCLUDED_REFERABLEFACTORY_H
#define INCLUDED_REFERABLEFACTORY_H
#include "core/Referable.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace core
{

class ReferableFactory : public ReferenceCounted
{
public:
	typedef Referable* (*CreationFunc)(const void*);

public:
	//! Initialize the global referable factory
	LUX_API static void Initialize();

	//! Access the global referable factory
	LUX_API static ReferableFactory* Instance();

	//! Destroys the global referable factory
	LUX_API static void Destroy();

	LUX_API ReferableFactory();
	LUX_API ~ReferableFactory();

	//! Register a new type
	/**
	The type name, resource type, type id are automatic read from the object.
	\param prototype The new prototype object for this type, every new object is cloned from this one
	*/
	LUX_API void RegisterType(Name name, CreationFunc create);

	//! Unregister a type
	/**
	\param type The main type
	\param subType The sub type
	*/
	LUX_API void UnregisterType(Name type);

	//! Create a new object
	/**
	\param type The type
	\param data Additinal data to pass to constructor, depends on type.
	\return The new object
	*/
	LUX_API StrongRef<Referable> Create(Name type, const void* data = nullptr);
	StrongRef<Referable> Create(const char* type, const void* data = nullptr)
	{
		return Create(Name(type), data);
	}

	//! The total number of types
	/**
	\return The total number of types
	*/
	LUX_API size_t GetTypeCount() const;

	//! Get a list of all type names.
	LUX_API core::Array<core::Name> GetTypeNames() const;

private:
	struct ReferableType
	{
		CreationFunc create;

		ReferableType() :
			create(nullptr)
		{
		}
		ReferableType(CreationFunc c) :
			create(c)
		{
		}
	};

private:
	core::HashMap<Name, ReferableType> m_Types;
};

}
}

#endif // !INCLUDED_IREFERABLEFACTORY_H
