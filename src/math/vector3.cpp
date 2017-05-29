#include "math/vector3.h"

namespace lux
{
namespace core
{

const Type Type::Vector3(new TypeInfoTemplate<math::vector3f>("vector3", true));
const Type Type::Vector3Int(new TypeInfoTemplate<math::vector3i>("vector3i", true));

} // namespace core
} // namespace lux
