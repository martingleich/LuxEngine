#include "math/vector2.h"

namespace lux
{
namespace core
{

const Type Type::Vector2(new TypeInfoTemplate<math::vector2f>("vector2", true));
const Type Type::Vector2Int(new TypeInfoTemplate<math::vector2i>("vector2i", true));

} // namespace core
} // namespace lux
