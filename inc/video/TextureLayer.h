#ifndef INCLUDED_SMATERIALLAYER_H
#define INCLUDED_SMATERIALLAYER_H
#include "core/lxTypes.h"
#include "video/BaseTexture.h"

namespace lux
{
namespace video
{

//! How are texturecoordinates outside of [0,1] handeled
enum class ETextureRepeat
{
	Wrap = 0,
	Mirror,
	Clamp,
	MirrorOnce,
};

//! A single texture layer in a material
class TextureLayer
{
public:
	struct RepeatMode
	{
		ETextureRepeat u;
		ETextureRepeat v;
		RepeatMode& operator=(ETextureRepeat r)
		{
			u = v = r;
			return *this;
		}

		bool operator==(RepeatMode other) const
		{
			return u == other.u && v == other.v;
		}

		bool operator!=(RepeatMode other) const
		{
			return !(*this == other);
		}

		RepeatMode() : u(ETextureRepeat::Wrap), v(ETextureRepeat::Wrap)
		{
		}
		RepeatMode(ETextureRepeat r) : u(r), v(r)
		{
		}
		RepeatMode(ETextureRepeat _u, ETextureRepeat _v) : u(_u), v(_v)
		{
		}
	};

public:
	StrongRef<BaseTexture>  texture;
	RepeatMode repeat;

	inline bool operator!=(const TextureLayer& other) const
	{
		return texture != other.texture || repeat != other.repeat;
	}

	inline bool operator==(const TextureLayer& other) const
	{
		return !(other != *this);
	}
};

} // namespace video

namespace core
{
template<> inline Type GetTypeInfo<video::TextureLayer>() { return Type::Texture; }
} // namespace core
} // namespace lux

#endif // !INCLUDED_SMATERIALLAYER_H
