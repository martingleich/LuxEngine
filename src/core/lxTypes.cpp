#include "core/lxTypes.h"

namespace lux
{
namespace core
{

const Type Type::Unknown(new TypeInfoVirtual("unknown", 0, 1, true));

namespace Types
{
Type Integer()
{
	static const Type t(new TypeInfoTemplate<int>("int"));
	return t;
}

Type U32()
{
	static const Type t(new TypeInfoTemplate<u32>("u32"));
	return t;
}

Type Float()
{
	static const Type t(new TypeInfoTemplate<float>("float"));
	return t;
}

Type Boolean()
{
	static const Type t(new TypeInfoTemplate<bool>("bool"));
	return t;
}

}
}
}