#include "core/ReferableFactory.h"

#include "core/ReferableFactoryImpl.h"

namespace lux
{
namespace core
{

static StrongRef<ReferableFactory> g_ReferableFactory;

void ReferableFactory::Initialize(ReferableFactory* refFactory)
{
	if(!refFactory)
		refFactory = LUX_NEW(ReferableFactoryImpl);

	g_ReferableFactory = refFactory;
}

ReferableFactory* ReferableFactory::Instance()
{
	return g_ReferableFactory;
}

void ReferableFactory::Destroy()
{
	g_ReferableFactory.Reset();
}

}
}
