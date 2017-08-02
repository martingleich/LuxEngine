#include "video/ColorSpaces.h"
#include "video/ColorInt.h"
#include "video/ColorFloat.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Colorf()
{
	static const Type t(new core::TypeInfoTemplate<video::Colorf>("colorF"));
	return t;
}
}
}
} // namespace lux
