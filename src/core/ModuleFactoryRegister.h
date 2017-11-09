#ifndef INCLUDED_MODULE_FACTORY_REGISTER_H
#define INCLUDED_MODULE_FACTORY_REGISTER_H
#include "core/ModuleFactory.h"

namespace lux
{
namespace core
{
namespace impl_moduleRegister
{
struct ModuleFactoryRegisterBlock;
void RegisterModuleFactoryBlock(ModuleFactoryRegisterBlock* block);
void RunAllModuleFactoryBlocks();

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
}

#define LUX_REGISTER_MODULE(module, name, class) \
static ::lux::ReferenceCounted* InternalCreatorFunc(const ::lux::core::ModuleInitData& data) { return LUX_NEW(class)(data); } \
static ::lux::core::impl_moduleRegister::ModuleFactoryRegisterBlock InternalModuleRegisterStaticObject(module, name, &InternalCreatorFunc);

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_MODULE_FACTORY_REGISTER_H