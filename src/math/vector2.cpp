#include "math/vector2.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Vector2f()
{
	static const Type t(new core::TypeInfoTemplate<math::vector2f>("vector2f"));
	return t;
}
Type Vector2i()
{
	static const Type t(new core::TypeInfoTemplate<math::vector2i>("vector2i"));
	return t;
}
}
}
} // namespace lux
