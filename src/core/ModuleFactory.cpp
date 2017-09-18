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

void ModuleFactory::AddModuleFactory(const String& module, const String& name, CreatorT creator)
{
	LX_CHECK_NULL_ARG(creator);

	for(auto& e : m_Entries) {
		if(e.module == module && e.name == name)
			throw core::InvalidArgumentException("name", "Name already used");
	}

	m_Entries.PushBack(Entry(module, name, creator));
}

core::Array<String> ModuleFactory::GetModuleFactories(const String& module)
{
	core::Array<String> out;
	for(auto& e : m_Entries) {
		if(e.module == module)
			out.PushBack(e.name);
	}

	return out;
}

StrongRef<ReferenceCounted> ModuleFactory::CreateModule(const String& module, const String& name, const ModuleInitData& data)
{
	for(auto& e : m_Entries) {
		if(e.module == module && e.name == name) {
			StrongRef<ReferenceCounted> out = e.creator(data);
			if(!out)
				throw core::RuntimeException("Factory function failed");
			return out;
		}
	}

	throw core::ObjectNotFoundException((module + "." + name).Data());
}

}
}
