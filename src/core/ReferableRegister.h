#ifndef INCLUDED_REFERABLE_REGISTER_H
#define INCLUDED_REFERABLE_REGISTER_H

namespace lux
{
class Referable;
namespace core
{
namespace impl
{
struct ReferableRegisterBlock;

// Implemented in ReferableFactoryImpl.cpp
void RegisterReferable(ReferableRegisterBlock* block);
void RunAllRegisterReferableFunctions();

struct ReferableRegisterBlock
{
	Referable* (*prototypeCreator)();
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(Referable* (*_prototypeCreator)()) :
		prototypeCreator(_prototypeCreator),
		nextBlock(nullptr)
	{
		RegisterReferable(this);
	}
};

}
}
}

#define LUX_REGISTER_REFERABLE_CLASS(class) \
static ::lux::Referable* InternalReferableRegisterPrototypeCreator() { return LUX_NEW(class); } \
static ::lux::core::impl::ReferableRegisterBlock InternalReferableRegisterStaticObject(&InternalReferableRegisterPrototypeCreator);
		
#define LUX_REGISTER_REFERABLE_CLASS_NAMED(name, class) \
static ::lux::Referable* InternalReferableRegisterPrototypeCreator_##name() { return LUX_NEW(class); } \
static ::lux::core::impl::ReferableRegisterBlock InternalReferableRegisterStaticObject_##name(&InternalReferableRegisterPrototypeCreator_##name);

#endif // #ifndef INCLUDED_REFERABLE_REGISTER_H