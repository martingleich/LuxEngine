#include "math/Vector3.h"

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
	static const Type t(new core::TypeInfoTemplate<math::Vector3I>("Vector3I"));
	return t;
}
}
}
} // namespace lux
