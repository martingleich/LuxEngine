#include "core/Referable.h"

namespace lux
{
namespace core
{
namespace Types
{

Type StrongID()
{
	static Type strongRef(new TypeInfoTemplate<core::ID>("strong_id"));
	return strongRef;
}

Type WeakID()
{
	static Type weakRef(new TypeInfoTemplate<core::ID>("weak_id"));
	return weakRef;
}

bool IsIDType(Type t)
{
	return t == StrongID() || t == WeakID();
}

}
}
}