#ifndef INCLUDED_MODULE_FACTORY_H
#define INCLUDED_MODULE_FACTORY_H
#include "core/lxArray.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace core
{

//! Structure passed to module factory function.
class ModuleInitData
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
	LUX_API void AddModuleFactory(const core::String& module, const core::String& name, CreatorT creator);

	//! Get all types of a module type.
	/**
	Returns all name parameters of a module.
	*/
	LUX_API core::Array<core::String> GetModuleFactories(const core::String& module);

	//! Create a instance of a module.
	/**
	Data must be a type matching the module, i.e. VideoDriverInitData for a VideoDriver.
	Return type is always non null.
	*/
	LUX_API StrongRef<ReferenceCounted> CreateModule(const core::String& module, const core::String& name, const ModuleInitData& data);

private:
	struct Entry
	{
		core::String module;
		core::String name;
		CreatorT creator;

		Entry(const core::String& _module,
			const core::String& _name,
			CreatorT _creator) :
			module(_module),
			name(_name),
			creator(_creator)
		{
		}
	};

	core::Array<Entry> m_Entries;
};

namespace impl_moduleRegister
{
struct ModuleFactoryRegisterBlock;
void LUX_API RegisterModuleFactoryBlock(ModuleFactoryRegisterBlock* block);
void LUX_API RunAllModuleFactoryBlocks();

struct ModuleFactoryRegisterBlock
{
	core::String module;
	core::String name;

	ModuleFactory::CreatorT creator;

	ModuleFactoryRegisterBlock* next;

	ModuleFactoryRegisterBlock(
		const core::String& _module,
		const core::String& _name,
		ModuleFactory::CreatorT _creator) :
		module(_module),
		name(_name),
		creator(_creator),
		next(nullptr)
	{
		RegisterModuleFactoryBlock(this);
	}
};

} // impl_moduleRegister
} // namespace core
} // namespace lux

#define LUX_REGISTER_MODULE(module, name, class) \
static ::lux::ReferenceCounted* InternalCreatorFunc(const ::lux::core::ModuleInitData& data) { return LUX_NEW(class)(data); } \
static ::lux::core::impl_moduleRegister::ModuleFactoryRegisterBlock InternalModuleRegisterStaticObject(module, name, &InternalCreatorFunc);

#endif // #ifndef INCLUDED_MODULE_FACTORY_H