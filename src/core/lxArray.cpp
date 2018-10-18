#include "core/lxArray.h"

namespace lux
{
namespace core
{

StringView MakeArrayTypeName(Type baseType)
{
	auto baseName = baseType.GetName();
	auto length = baseName.Size();
	char* ptr = LUX_NEW_ARRAY(char, length + 3);
	memcpy(ptr, baseName.Data(), baseName.Size());
	memcpy(ptr + baseName.Size(), "[]", 3);
	return StringView(ptr, length+2);
}

} // namespace core
} // namespace lux