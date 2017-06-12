#ifndef INCLUDED_SPRITEBANK_IMPL_H
#define INCLUDED_SPRITEBANK_IMPL_H
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
		math::rectf rect;
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

	SpriteBank::Sprite AddSprite(Texture* texture, const math::rect<u16>& rect);
	SpriteBank::Sprite AddTextureAsSprite(Texture* texture);
	SpriteBank::Sprite AddAnimatedSprite(SpriteBank::Sprite first, SpriteBank::Sprite last, u32 frameTime);
	void Clear();
	bool GetSprite(SpriteBank::Sprite index, u32 time, bool looped, math::rectf*& outCoords, Texture*& outTex);
	bool DrawSprite(SpriteBank::Sprite index, const math::vector2i& pos, u32 time = 0, bool looped = true, bool centered = false);

private:
	core::array<Sprite> m_Sprites;
	core::array<AnimatedSprite> m_AnimatedSprites;
	core::array<StrongRef<Texture>> m_Textures;
};

}
}

#endif // #ifndef INCLUDED_SPRITEBANK_IMPL_H