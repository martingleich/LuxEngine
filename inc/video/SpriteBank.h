#ifndef INCLUDED_LUX_SPRITEBANK_H
#define INCLUDED_LUX_SPRITEBANK_H
#include "core/ReferenceCounted.h"
#include "math/Rect.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{
class Texture;

//! A sprite bank
/**
A sprite bank saves multiple sprites, static and animated.
With sprite you can speed up rendering, because a lot of sprites
can be contained inside a single texture, and less state changes are necessary
*/
class SpriteBank : public ReferenceCounted
{
public:
	//! A handle to a single sprite
	struct Sprite
	{
		LUX_API static const Sprite INVALID; //!< The invalid sprite

		int id; //!< The id of the sprite

		Sprite() :
			id(0)
		{
		}
		explicit Sprite(int i) :
			id(i)
		{
		}

		//! Is the sprite animated
		inline bool IsAnimated() const
		{
			return id < 0;
		}
		//! Is the sprite static
		inline bool IsStatic() const
		{
			return id > 0;
		}
		//! Is the sprite valid
		inline bool IsValid() const
		{
			return id != 0;
		}

		//! Defines an ordering operation on sprites
		bool operator<(const Sprite& other) const
		{
			if(id < 0)
				return id > other.id;
			if(id > 0)
				return id < other.id;
			return false;
		}
	};

private:
	struct SpriteEntry
	{
		int	textureID;
		math::RectF rect;
	};

	struct AnimatedSpriteEntry
	{
		int firstSprite;
		int lastSprite;

		int frameTime;
	};

public:
	LUX_API SpriteBank();
	LUX_API ~SpriteBank();

	//! Add a new sprite
	/**
	\param texture The texture which contains the sprite
	\param rect The part of the texture which contatins the sprite
	\return A handle to the new sprite
	*/
	LUX_API Sprite AddSprite(Texture* texture, const math::RectI& rect);

	//! Add a new sprite
	/**
	Add a complete texture as new sprite
	\param texture Thew texture which contains the sprite
	\return A handle to the new sprite
	*/
	LUX_API Sprite AddTextureAsSprite(Texture* texture);

	//! Add a new animated sprite
	/**
	An animated sprite shows a sprite depending on the given time.
	sprite are played in order of the sprites, which is the order in which they were created
	\param first The first sprite in the animation
	\param last The last sprite in the animation
	\param frameTime The length of a single frame in milliseconds
	\return A handle to the new sprite
	*/
	LUX_API Sprite AddAnimatedSprite(Sprite first, Sprite last, int frameTime);

	//! Delete all sprites from the bank
	LUX_API void Clear();

	//! Retrieve information about a single sprite
	/**
	\param sprite A handle to the sprite
	\param time The current animation time in milliseconds
	\param looped Should the animation be looped, or stop on the last frame
	\param [out] outCoords The texturecoordinates to use for rendering
	\param [out] outText The texture to set for rendering
	\return Could all information be retrieved
	*/
	LUX_API bool GetSprite(
		Sprite sprite,
		int time, bool looped,
		math::RectF*& outCoords, Texture*& outTex);

private:
	int GetTextureId(Texture* texture);

private:
	core::Array<SpriteEntry> m_Sprites;
	core::Array<AnimatedSpriteEntry> m_AnimatedSprites;
	core::Array<StrongRef<Texture>> m_Textures;
};

}
}
#endif // INCLUDED_LUX_SPRITEBANK_H
