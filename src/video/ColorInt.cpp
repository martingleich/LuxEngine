#include "video/ColorInt.h"

namespace lux
{
namespace core
{
namespace Types
{

Type Color()
{
	static const Type t(LUX_NEW(core::TypeInfoTemplate<video::Color>)("Color", true));
	return t;
}

}
}
} // namespace lux