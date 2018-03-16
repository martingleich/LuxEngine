#include "math/Quaternion.h"

namespace lux
{

namespace core
{
namespace Types
{
Type QuaternionF()
{
	static const Type t(LUX_NEW(core::TypeInfoTemplate<math::QuaternionF>)("QuaternionF", true));
	return t;
}
}
}
} // namespace lux