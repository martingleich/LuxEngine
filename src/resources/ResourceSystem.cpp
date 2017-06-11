#include "resources/ResourceSystem.h"

#include "resources/ResourceSystemImpl.h"

namespace lux
{
namespace core
{

static StrongRef<ResourceSystem> g_ResourceSystem;

void ResourceSystem::Initialize(ResourceSystem* resSys)
{
	if(!resSys)
		resSys = LUX_NEW(ResourceSystemImpl);

	g_ResourceSystem = resSys;
}

ResourceSystem* ResourceSystem::Instance()
{
	return g_ResourceSystem;
}

void ResourceSystem::Destroy()
{
	g_ResourceSystem.Reset();
}

}
}