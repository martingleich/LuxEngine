#ifndef INCLUDED_SMATERIALLAYER_H
#define INCLUDED_SMATERIALLAYER_H
#include "core/lxTypes.h"
#include "core/lxFormat.h"
#include "video/BaseTexture.h"
#include "video/VideoEnums.h"

namespace lux
{
namespace video
{

//! A single texture layer in a material
class TextureLayer
{
public:
	struct RepeatMode
	{
		video::Color border;
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
	StrongRef<BaseTexture> texture;
	RepeatMode repeat;

	TextureLayer() {}
	explicit TextureLayer(BaseTexture* tex) :
		texture(tex)
	{}

	inline bool operator!=(const TextureLayer& other) const
	{
		return texture != other.texture || repeat != other.repeat;
	}

	inline bool operator==(const TextureLayer& other) const
	{
		return !(other != *this);
	}
};

inline void fmtPrint(format::Context& ctx, const TextureLayer& layer, format::Placeholder& placeholder)
{
	ctx.AddTerminatedSlice("<texturelayer:");
	format::fmtPrint(ctx, (const void*)&layer, placeholder);
	ctx.AddTerminatedSlice(">");
}

} // namespace video

namespace core
{
namespace Types
{
LUX_API Type Texture();
}

template<> struct TemplType<video::TextureLayer> { static Type Get() { return Types::Texture(); } };

} // namespace core
} // namespace lux

#endif // !INCLUDED_SMATERIALLAYER_H
