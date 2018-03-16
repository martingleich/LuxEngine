#include "video/TextureLayer.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Texture()
{
	static const Type t(LUX_NEW(core::TypeInfoTemplate<video::TextureLayer>)("Texture"));
	return t;
}
}
}
} // namespace lux
