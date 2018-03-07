#include "core/lxArray.h"

namespace lux
{
namespace core
{

const char* MakeArrayTypeName(Type baseType)
{
	auto baseName = baseType.GetName();
	auto length = strlen(baseName);
	char* ptr = new char[length + 3];
	strcpy(ptr, baseName);
	strcpy(ptr + length, "[]");
	return ptr;
}

Type TestCall()
{
	return TemplType<core::Array<u32>>::Get();
}
} // namespace core
} // namespace lux