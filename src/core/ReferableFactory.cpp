#include "core/ReferableFactory.h"

#include "core/ReferableFactoryImpl.h"

namespace lux
{
namespace core
{

static StrongRef<ReferableFactory> g_ReferableFactory;
ReferableFactory* ReferableFactory::Instance()
{
	if(!g_ReferableFactory)
		g_ReferableFactory = LUX_NEW(ReferableFactoryImpl);
	return g_ReferableFactory;
}

void ReferableFactory::Destroy()
{
	g_ReferableFactory.Reset();
}

}
}
