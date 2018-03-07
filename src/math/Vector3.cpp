#include "math/Vector3.h"

namespace lux
{

namespace core
{
namespace Types
{
Type Vector3F()
{
	static const Type t(new core::TypeInfoTemplate<math::Vector3F>("Vector3F", true));
	return t;
}
Type Vector3I()
{
	static const Type t(new core::TypeInfoTemplate<math::Vector3I>("Vector3I", true));
	return t;
}
}
}
} // namespace lux
