#include "core/ModuleFactory.h"

namespace lux
{
namespace core
{
static StrongRef<ModuleFactory> g_Factory;
void ModuleFactory::Initialize()
{
	if(!g_Factory)
		g_Factory = LUX_NEW(ModuleFactory);
}

ModuleFactory* ModuleFactory::Instance()
{
	return g_Factory;
}

void ModuleFactory::Destroy()
{
	g_Factory = nullptr;
}

void ModuleFactory::AddModuleFactory(const core::String& module, const core::String& name, CreatorT creator)
{
	LX_CHECK_NULL_ARG(creator);

	for(auto& e : m_Entries) {
		if(e.module == module && e.name == name)
			throw core::GenericInvalidArgumentException("name", "Name already used");
	}

	m_Entries.PushBack(Entry(module, name, creator));
}

core::Array<core::String> ModuleFactory::GetModuleFactories(const core::String& module)
{
	core::Array<core::String> out;
	for(auto& e : m_Entries) {
		if(e.module == module)
			out.PushBack(e.name);
	}

	return out;
}

StrongRef<ReferenceCounted> ModuleFactory::CreateModule(const core::String& module, const core::String& name, const ModuleInitData& data)
{
	for(auto& e : m_Entries) {
		if(e.module == module && e.name == name) {
			StrongRef<ReferenceCounted> out = e.creator(data);
			if(!out)
				throw core::FactoryCreateException(module.Data(), "Creation of module failed");
			return out;
		}
	}

	throw core::ObjectNotFoundException((module + "." + name).Data());
}

} // namespace core

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
		core::ModuleFactory::Instance()->AddModuleFactory(block->module, block->name, block->creator);
}

} // namespace impl_moduleRegister
} // namespace lux
