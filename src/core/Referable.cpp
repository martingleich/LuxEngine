#include "core/Referable.h"

namespace lux
{
namespace core
{
namespace Types
{
static Type strongRef(new TypeInfoTemplate<lux::StrongRef<Referable>>("strong_ref"));

Type StrongRef()
{
	return strongRef;
}

}
}
}