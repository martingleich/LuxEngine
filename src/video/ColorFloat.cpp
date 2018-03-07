#include "video/ColorSpaces.h"
#include "video/ColorInt.h"
#include "video/ColorFloat.h"

namespace lux
{
namespace core
{
namespace Types
{
Type ColorF()
{
	static const Type t(new core::TypeInfoTemplate<video::ColorF>("ColorF", true));
	return t;
}
}
}
} // namespace lux
