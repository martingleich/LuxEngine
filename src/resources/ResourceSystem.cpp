#include "resources/ResourceSystem.h"

#include "resources/ResourceSystemImpl.h"

namespace lux
{
namespace core
{

static StrongRef<ResourceSystem> g_ResourceSystem;
ResourceSystem* ResourceSystem::Instance()
{
	if(!g_ResourceSystem)
		g_ResourceSystem = LUX_NEW(ResourceSystemImpl);
	return g_ResourceSystem;
}

void ResourceSystem::Destroy()
{
	g_ResourceSystem.Reset();
}

}
}