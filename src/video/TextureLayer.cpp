#include "video/TextureLayer.h"

namespace lux
{
namespace core
{
namespace Types
{
Type Texture()
{
	static const Type t(new core::TypeInfoTemplate<video::TextureLayer>("texture"));
	return t;
}
}
}
} // namespace lux
