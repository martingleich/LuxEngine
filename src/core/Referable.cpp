#include "core/Referable.h"

namespace lux
{
namespace core
{
namespace Types
{
static Type strongRef(new TypeInfoTemplate<core::ID>("strong_ref"));
static Type weakRef(new TypeInfoTemplate<core::ID>("strong_ref"));

Type StrongRef()
{
	return strongRef;
}
Type WeakRef()
{
	return weakRef;
}

}
}
}