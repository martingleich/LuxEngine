#ifndef INCLUDED_LUX_SPRITEBANK_IMPL_H
#define INCLUDED_LUX_SPRITEBANK_IMPL_H
#include "video/SpriteBank.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{
class VideoDriver;

class SpriteBankImpl : public SpriteBank
{
private:
	struct Sprite
	{
		u16	textureID;
		math::RectF rect;
	};

	struct AnimatedSprite
	{
		u32 firstSprite;
		u32 lastSprite;

		u32 frameTime;
	};
public:
	SpriteBankImpl();
	virtual ~SpriteBankImpl();

	SpriteBank::Sprite AddSprite(Texture* texture, const math::Rect<u16>& rect);
	SpriteBank::Sprite AddTextureAsSprite(Texture* texture);
	SpriteBank::Sprite AddAnimatedSprite(SpriteBank::Sprite first, SpriteBank::Sprite last, u32 frameTime);
	void Clear();
	bool GetSprite(SpriteBank::Sprite index, u32 time, bool looped, math::RectF*& outCoords, Texture*& outTex);
	bool DrawSprite(SpriteBank::Sprite index, const math::Vector2I& pos, u32 time = 0, bool looped = true, bool centered = false);

private:
	core::Array<Sprite> m_Sprites;
	core::Array<AnimatedSprite> m_AnimatedSprites;
	core::Array<StrongRef<Texture>> m_Textures;
};

}
}

#endif // #ifndef INCLUDED_LUX_SPRITEBANK_IMPL_H