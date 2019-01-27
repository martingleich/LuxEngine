#include "video/SpriteBank.h"
#include "video/VideoDriver.h"
#include "video/Texture.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace video
{

const SpriteBank::Sprite SpriteBank::Sprite::INVALID = SpriteBank::Sprite(0);

static int GetStaticSpriteId(SpriteBank::Sprite sprite)
{
	lxAssert(sprite.id > 0);
	return sprite.id-1;
}

SpriteBank::SpriteBank()
{
}

SpriteBank::~SpriteBank()
{
}

SpriteBank::Sprite SpriteBank::AddSprite(Texture* texture, const math::RectI& rect)
{
	LX_CHECK_NULL_ARG(texture);

	auto dim = texture->GetSize();
	SpriteEntry sprite;
	sprite.textureID = GetTextureId(texture);
	sprite.rect.top = rect.top / (float)dim.height;
	sprite.rect.bottom = rect.bottom / (float)dim.height;
	sprite.rect.left = rect.left / (float)dim.width;
	sprite.rect.right = rect.right / (float)dim.width;

	m_Sprites.PushBack(sprite);
	return SpriteBank::Sprite(m_Sprites.Size());
}

SpriteBank::Sprite SpriteBank::AddTextureAsSprite(Texture* texture)
{
	LX_CHECK_NULL_ARG(texture);
	auto dim = texture->GetSize();
	return AddSprite(texture, math::RectI(0, 0, dim.width, dim.height));
}

SpriteBank::Sprite SpriteBank::AddAnimatedSprite(
	SpriteBank::Sprite first, SpriteBank::Sprite last, float frameTime, bool looped)
{
	LX_CHECK_BOUNDS(first.id, 1, m_Sprites.Size() + 1);
	LX_CHECK_BOUNDS(last.id, 1, m_Sprites.Size() + 1);
	if(first.id >= last.id)
		throw core::GenericInvalidArgumentException("first, last", "first must be smaller than last");

	AnimatedSpriteEntry sprite;
	sprite.firstSprite = GetStaticSpriteId(first);
	sprite.lastSprite = GetStaticSpriteId(last);
	sprite.time = frameTime;
	sprite.looped = looped;

	m_AnimatedSprites.PushBack(sprite);
	return SpriteBank::Sprite(-m_AnimatedSprites.Size());
}

void SpriteBank::Clear()
{
	m_Sprites.Clear();
	m_AnimatedSprites.Clear();
	m_Textures.Clear();
}

float SpriteBank::GetProperSpriteTime(Sprite sprite, float time)
{
	if(!sprite.IsAnimated())
		return 0;

	auto& a = m_AnimatedSprites[-sprite.id - 1];
	if(time >= a.time) {
		if(a.looped)
			return std::fmodf(time, a.time);
		return a.time;
	} else {
		return time;
	}
}

bool SpriteBank::GetSprite(
	SpriteBank::Sprite sprite,
	float time, math::RectF*& outCoords, Texture*& outTex)
{
	if(sprite.id < 0) {
		int idx = -sprite.id - 1;
		if(idx >= m_AnimatedSprites.Size())
			return false;

		auto& a = m_AnimatedSprites[idx];

		time = GetProperSpriteTime(sprite, time);
		int f = int((time / a.time) * (a.lastSprite - a.firstSprite));
		// Might overflow
		if(f > a.lastSprite - a.firstSprite) {
			if(a.looped)
				f = 0;
			else
				f = a.lastSprite-a.firstSprite;
		}

		auto& s = m_Sprites[a.firstSprite + f];
		outCoords = &s.rect;
		outTex = m_Textures[s.textureID];
	} else if(sprite.id > 0) {
		auto idx = GetStaticSpriteId(sprite);
		if(idx >= m_Sprites.Size())
			return false;

		auto& s = m_Sprites[idx];
		outCoords = &s.rect;
		outTex = m_Textures[s.textureID];
	} else {
		outCoords = nullptr;
		outTex = nullptr;
		return false;
	}

	return true;
}

int SpriteBank::GetTextureId(Texture* texture)
{
	for(int i = 0; i < m_Textures.Size(); ++i) {
		if(m_Textures[i] == texture)
			return i;
	}
	m_Textures.PushBack(texture);
	return m_Textures.Size()-1;
}

} // !namespace video
} // !namespace lux
