#ifndef INCLUDED_MODULE_FACTORY_H
#define INCLUDED_MODULE_FACTORY_H
#include "core/lxArray.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace core
{

//! Structure passed to module factory function.
struct ModuleInitData
{
public:
	// Virtual destructor to make dynamic_cast possible
	virtual ~ModuleInitData() {}

	void* user; //!< Pointer to user data.
};

//! Factory to create modules
class ModuleFactory : public ReferenceCounted
{
public:
	//!< Prototype of factory function
	typedef ReferenceCounted* (*CreatorT)(const ModuleInitData& data);

public:
	LUX_API static void Initialize();
	LUX_API static ModuleFactory* Instance();
	LUX_API static void Destroy();

	//! Add a new factory function.
	/**
	The combination of module and name must be unique.
	*/
	LUX_API void AddModuleFactory(const String& module, const String& name, CreatorT creator);

	//! Get all types of a module type.
	/**
	Returns all name parameters of a module.
	*/
	LUX_API core::Array<String> GetModuleFactories(const String& module);

	//! Create a instance of a module.
	/**
	Data must be a type matching the module, i.e. VideoDriverInitData for a VideoDriver.
	Return type is always non null.
	*/
	LUX_API StrongRef<ReferenceCounted> CreateModule(const String& module, const String& name, const ModuleInitData& data);

private:
	struct Entry
	{
		String module;
		String name;
		CreatorT creator;

		Entry(const String& _module,
			const String& _name,
			CreatorT _creator) :
			module(_module),
			name(_name),
			creator(_creator)
		{
		}
	};

	core::Array<Entry> m_Entries;
};

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_MODULE_FACTORY_H