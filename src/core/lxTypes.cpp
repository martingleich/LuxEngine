#include "core/lxTypes.h"
#include "core/lxFormat.h"

namespace lux
{
namespace core
{

const Type Type::Unknown(LUX_NEW(TypeInfoVirtual)("unknown", 0, 1, true));

namespace Types
{

Type Integer()
{
	static const Type intT(LUX_NEW(TypeInfoTemplate<int>)("int"));
	return intT;
}

Type UInteger()
{
	static const Type uintT(LUX_NEW(TypeInfoTemplate<unsigned int>)("uint"));
	return uintT;
}

Type Byte()
{
	static const Type byteT(LUX_NEW(TypeInfoTemplate<u8>)("byte"));
	return byteT;
}

Type Float()
{
	static const Type floatT(LUX_NEW(TypeInfoTemplate<float>)("float"));
	return floatT;
}

Type Boolean()
{
	static const Type boolT(LUX_NEW(TypeInfoTemplate<bool>)("bool"));
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