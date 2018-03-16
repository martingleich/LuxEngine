#include "core/Referable.h"

namespace lux
{
namespace core
{
namespace Types
{

Type StrongID()
{
	static Type strongRef(LUX_NEW(TypeInfoTemplate<core::ID>)("strong_id"));
	return strongRef;
}

Type WeakID()
{
	static Type weakRef(LUX_NEW(TypeInfoTemplate<core::ID>)("weak_id"));
	return weakRef;
}

bool IsIDType(Type t)
{
	return t == StrongID() || t == WeakID();
}

}
}
}