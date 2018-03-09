#include "core/lxTypes.h"
#include "core/lxFormat.h"

namespace lux
{
namespace core
{

const Type Type::Unknown(new TypeInfoVirtual("unknown", 0, 1, true));

namespace Types
{

Type Integer()
{
	static const Type intT(new TypeInfoTemplate<int>("int"));
	return intT;
}

Type U32()
{
	static const Type u32T(new TypeInfoTemplate<u32>("u32"));
	return u32T;
}

Type Byte()
{
	static const Type byteT(new TypeInfoTemplate<u8>("byte"));
	return byteT;
}

Type Float()
{
	static const Type floatT(new TypeInfoTemplate<float>("float"));
	return floatT;
}

Type Boolean()
{
	static const Type boolT(new TypeInfoTemplate<bool>("bool"));
	return boolT;
}

}

void TypeInfo::FmtPrint(format::Context& ctx, const void* p, format::Placeholder& placeholder) const
{
	ctx.AddTerminatedSlice("<");
	ctx.AddTerminatedSlice(GetName());
	ctx.AddTerminatedSlice(":");
	format::fmtPrint(ctx, p, placeholder);
	ctx.AddTerminatedSlice(">");
}

}
}