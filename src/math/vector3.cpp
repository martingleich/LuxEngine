#include "math/vector3.h"

namespace lux
{

namespace core
{
namespace Types
{
Type Vector3f()
{
	static const Type t(new core::TypeInfoTemplate<math::Vector3F>("vector3f"));
	return t;
}
Type Vector3i()
{
	static const Type t(new core::TypeInfoTemplate<math::vector3i>("vector3i"));
	return t;
}
}
}
} // namespace lux
