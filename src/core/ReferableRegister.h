#ifndef INCLUDED_REFERABLE_REGISTER_H
#define INCLUDED_REFERABLE_REGISTER_H
#include "resources/Resource.h"

namespace lux
{

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
	Referable* (*creator)(const void*);
	ReferableRegisterBlock* nextBlock;

	ReferableRegisterBlock(Name t, Referable* (*_creator)(const void*)) :
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
static ::lux::Referable* InternalCreatorFunc(const void*) { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject(type, &InternalCreatorFunc);

#define LUX_REGISTER_REFERABLE_CLASS_NAMED(name, type, class) \
static ::lux::Referable* InternalCreatorFunc_##name(const void*) { return LUX_NEW(class); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject_##name(type, &InternalCreatorFunc_##name);

#define LUX_REGISTER_RESOURCE_CLASS(type, class) \
static ::lux::Referable* InternalCreatorFunc(const void* origin) { return LUX_NEW(class)(origin?*(lux::core::ResourceOrigin*)origin:lux::core::ResourceOrigin()); } \
static ::lux::core::impl_referableRegister::ReferableRegisterBlock InternalReferableRegisterStaticObject(type, &InternalCreatorFunc);

#endif // #ifndef INCLUDED_REFERABLE_REGISTER_H