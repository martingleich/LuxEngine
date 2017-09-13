#include "math/Quaternion.h"

namespace lux
{

namespace core
{
namespace Types
{
Type QuaternionF()
{
	static const Type t(new core::TypeInfoTemplate<math::QuaternionF>("quaternionf"));
	return t;
}
}
}
} // namespace lux