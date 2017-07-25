#ifndef INCLUDED_REFERABLE_REGISTER_H
#define INCLUDED_REFERABLE_REGISTER_H

namespace lux
{
class Referable;
namespace core
{
namespace impl_referableRegister
{
struct ReferableRegisterBlock;

// Implemented in ReferableFactoryImpl.cpp
void RegisterReferableBlock(ReferableRegisterBlock* block);
void RunAllRegisterReferableFunctions();

struct ReferableRegisterBlock
{
	Name type;
	Referable* (*creator)();
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(Name t, Referable* (*_creator)()) :
		type(t),
		creator(_creator),
		nextBlock(nullptr)
	{
		RegisterReferableBlock(this);
	}
};

}
}
}

#define LUX_REGISTER_REFERABLE_CLASS(type, class) \
static ::lux::Referable* InternalCreatorFunc() { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject(type, &InternalCreatorFunc);
		
#define LUX_REGISTER_REFERABLE_CLASS_NAMED(name, type, class) \
static ::lux::Referable* InternalCreatorFunc_##name() { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject_##name(type, &InternalCreatorFunc_##name);

#endif // #ifndef INCLUDED_REFERABLE_REGISTER_H