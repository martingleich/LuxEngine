#include "math/Vector2.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Vector2F()
{
	static const Type t(new core::TypeInfoTemplate<math::Vector2F>("Vector2I", true));
	return t;
}
Type Vector2I()
{
	static const Type t(new core::TypeInfoTemplate<math::Vector2I>("Vector2F", true));
	return t;
}
}
}
} // namespace lux
