#include "SpriteBankImpl.h"
#include "video/VideoDriver.h"
#include "video/Texture.h"
#include "core/lxAlgorithm.h"

namespace lux
{
namespace video
{

const SpriteBank::Sprite SpriteBank::Sprite::INVALID = SpriteBank::Sprite(0);

SpriteBankImpl::SpriteBankImpl(VideoDriver* videoDriver)
{
	m_Driver = videoDriver;
}

SpriteBankImpl::~SpriteBankImpl()
{
	Clear();
}

SpriteBank::Sprite SpriteBankImpl::AddSprite(Texture* texture, const math::rect<u16>& rect)
{
	if(!texture)
		return 0;

	auto it = core::Linear_Search(texture, m_Textures.First(), m_Textures.End());
	if(it == m_Textures.End())
		it = m_Textures.Push_Back(texture);

	math::dimension2du dim = texture->GetDimension();
	Sprite sprite;
	sprite.textureID = (u16)core::IteratorDistance(m_Textures.First(), it);
	sprite.rect.Top = rect.Top / (float)dim.height;
	sprite.rect.Bottom = rect.Bottom / (float)dim.height;
	sprite.rect.Left = rect.Left / (float)dim.width;
	sprite.rect.Right = rect.Right / (float)dim.width;

	return SpriteBank::Sprite((s32)core::IteratorDistance(m_Sprites.First(), m_Sprites.Push_Back(sprite)) + 1);
}

SpriteBank::Sprite SpriteBankImpl::AddTextureAsSprite(Texture* texture)
{
	if(!texture)
		return 0;

	auto it = core::Linear_Search(texture, m_Textures.First(), m_Textures.End());
	if(it == m_Textures.End())
		it = m_Textures.Push_Back(texture);

	Sprite sprite;
	sprite.textureID = (u16)core::IteratorDistance(m_Textures.First(), it);
	sprite.rect = math::rectf(0.0f, 0.0f, 1.0f, 1.0f);

	return SpriteBank::Sprite((u16)core::IteratorDistance(m_Sprites.First(), m_Sprites.Push_Back(sprite)) + 1);
}

SpriteBank::Sprite SpriteBankImpl::AddAnimatedSprite(SpriteBank::Sprite first, SpriteBank::Sprite last, u32 frameTime)
{
	first.id -= 1;
	last.id -= 1;

	if((u32)first.id > m_Sprites.Size())
		first.id = (s32)m_Sprites.Size();
	if((u32)last.id > m_Sprites.Size())
		last.id = (s32)m_Sprites.Size();

	if(first.id >= last.id)
		return 0;

	AnimatedSprite sprite;
	sprite.firstSprite = first.id;
	sprite.lastSprite = last.id;
	sprite.frameTime = frameTime;

	return SpriteBank::Sprite(-1 * ((s32)core::IteratorDistance(m_AnimatedSprites.First(), m_AnimatedSprites.Push_Back(sprite)) + 1));
}

void SpriteBankImpl::Clear()
{
	m_Sprites.Clear();
	m_AnimatedSprites.Clear();
	m_Textures.Clear();
}

bool SpriteBankImpl::GetSprite(SpriteBank::Sprite sprite, u32 Time, bool Looped, math::rectf*& outCoords, Texture*& outTex)
{
	if(sprite.id < 0) {
		sprite.id = -sprite.id - 1;
		if(sprite.id >= (int)m_AnimatedSprites.Size())
			return false;

		AnimatedSprite& a = m_AnimatedSprites[sprite.id];

		u32 f = Time / a.frameTime;
		if(f > a.lastSprite - a.firstSprite)
			if(Looped)
				f = f % (a.lastSprite - a.firstSprite);
			else
				f = a.lastSprite;

		Sprite& s = m_Sprites[a.firstSprite + f];
		outCoords = &s.rect;
		outTex = m_Textures[s.textureID];
	} else if(sprite.id > 0) {
		sprite.id -= 1;
		if(sprite.id >= (int)m_Sprites.Size())
			return false;

		Sprite& s = m_Sprites[sprite.id];
		outCoords = &s.rect;
		outTex = m_Textures[s.textureID];
	} else {
		outCoords = nullptr;
		outTex = nullptr;
		return false;
	}

	return true;
}

bool SpriteBankImpl::DrawSprite(SpriteBank::Sprite index, const math::vector2i& pos, u32 time, bool looped, bool centered)
{
	math::rectf* r;
	Texture* t;
	if(GetSprite(index.id, time, looped, r, t) == false)
		return false;

	math::vector2f dia = math::vector2f(r->GetWidth()*t->GetDimension().width, r->GetHeight()*t->GetDimension().height);
	math::recti loc;
	if(centered)
		loc = math::recti(pos.x - (int)dia.x / 2, pos.y - (int)dia.y / 2, pos.x + (int)dia.x / 2, pos.y + (int)dia.y / 2);
	else
		loc = math::recti(pos.x, pos.y, pos.x + (int)dia.x, pos.y + (int)dia.y);
	return m_Driver->Draw2DImage(t, loc, *r,
		video::Color::White, true);
}

} // !namespace video
} // !namespace lux