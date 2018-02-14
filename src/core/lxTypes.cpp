#include "core/lxTypes.h"

namespace lux
{
namespace core
{

const Type Type::Unknown(new TypeInfoVirtual("unknown", 0, 1, true));

namespace Types
{
static const Type intT(new TypeInfoTemplate<int>("int"));
static const Type u32T(new TypeInfoTemplate<u32>("u32"));
static const Type floatT(new TypeInfoTemplate<float>("float"));
static const Type boolT(new TypeInfoTemplate<bool>("bool"));

Type Integer()
{
	return intT;
}

Type U32()
{
	return u32T;
}

Type Float()
{
	return floatT;
}

Type Boolean()
{
	return boolT;
}

}
}
}