#include "core/ModuleFactoryRegister.h"

namespace lux
{
namespace core
{
namespace impl_moduleRegister
{
static ModuleFactoryRegisterBlock* g_FirstModuleBlock = nullptr;

void RegisterModuleFactoryBlock(ModuleFactoryRegisterBlock* block)
{
	block->next = g_FirstModuleBlock;
	g_FirstModuleBlock = block;
}

void RunAllModuleFactoryBlocks()
{
	for(auto block = g_FirstModuleBlock; block; block = block->next)
		ModuleFactory::Instance()->AddModuleFactory(block->module, block->name, block->creator);
}
}
} // namespace core
} // namespace lux