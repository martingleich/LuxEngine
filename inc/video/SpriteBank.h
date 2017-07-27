#ifndef INCLUDED_SPRITEBANK_H
#define INCLUDED_SPRITEBANK_H
#include "core/ReferenceCounted.h"
#include "math/Rect.h"

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
		static const Sprite INVALID; //!< The invalid sprite

		s32 id; //!< The id of the sprite

		Sprite() :
			id(0)
		{
		}
		Sprite(s32 i) :
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

public:
	//! Add a new sprite
	/**
	\param texture The texture which contains the sprite
	\param rect The part of the texture which contatins the sprite
	\return A handle to the new sprite
	*/
	virtual Sprite AddSprite(Texture* texture, const math::Rect<u16>& rect) = 0;

	//! Add a new sprite
	/**
	Add a complete texture as new sprite
	\param texture Thew texture which contains the sprite
	\return A handle to the new sprite
	*/
	virtual Sprite AddTextureAsSprite(Texture* texture) = 0;

	//! Add a new animated sprite
	/**
	An animated sprite shows a sprite depending on the given time.
	sprite are played in order of the sprites, which is the order in which they were created
	\param First The first sprite in the animation
	\param Last The last sprite in the animation
	\param FrameTime The length of a single frame in milliseconds
	\return A handle to the new sprite
	*/
	virtual Sprite AddAnimatedSprite(Sprite First, Sprite Last, u32 FrameTime) = 0;

	//! Delete all sprites from the bank
	virtual void Clear() = 0;

	//! Retrieve information about a single sprite
	/**
	\param sprite A handle to the sprite
	\param Time The current animation time in milliseconds
	\param Looped Should the animation be looped, or stop on the last frame
	\param [out] outCoords The texturecoordinates to use for rendering
	\param [out] outText The texture to set for rendering
	\return Could all information be retrieved
	*/
	virtual bool GetSprite(Sprite sprite, u32 Time, bool Looped, math::RectF*& outCoords, Texture*& outTex) = 0;

	//! Draws a single sprite
	/**
	Dont use this method to draw a lot of sprites.
	If you want to draw multiple sprites you should create a submesh
	and fill it with the data you get from SpriteBank::GetSprite
	\param sprite A handle to the sprite to draw
	\param Position Where to draw the sprite
	\param Time The current animation time in milliseconds
	\param Looped Should the animation be looped, or stop on the last frame
	\param Centered Should the sprite be centerd on the draw position or should it be drawn from upper-left down
	\return Could the sprite be drawn
	*/
	virtual bool DrawSprite(Sprite sprite, const math::Vector2I& Position, u32 Time = 0, bool Looped = true, bool Centered = false) = 0;
};

}
}
#endif // INCLUDED_SPRITEBANK_H
