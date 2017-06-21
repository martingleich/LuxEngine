#ifndef INCLUDED_LXRANDOM_H
#define INCLUDED_LXRANDOM_H
#include "ReferenceCounted.h"

#include "video/Color.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/angle.h"
#include "math/aabbox3d.h"
#include "math/rect.h"

#include <ctime>

namespace lux
{
namespace core
{

//! Random number engine.
/**
Implemented via 32-Bit Xor-Shift algorithm.
*/
class Randomizer
{
public:
	Randomizer()
	{
		ReSeed();
	}

	Randomizer(u32 seed)
	{
		ReSeed(seed);
	}

	static Randomizer& Instance()
	{
		static Randomizer r;
		return r;
	}

	//! Seed the random number generator, with the current time
	void ReSeed()
	{
		clock_t time = std::clock();
		u32 seed;
		// We don't care what exactly clock_t is,
		// real or integer or what size
		// We just want a pseudo random number from it.
		ifconst(sizeof(clock_t) == 8) {
			u32 s1, s2;
			memcpy(&s1, &time, 4);
			memcpy(&s2, ((u8*)&time) + 4, 4);
			seed = s1 ^ s2;
		} else {
			memcpy(&seed, &time, sizeof(clock_t));
		}

		ReSeed(seed);
	}

	//! Seed the random number generator.
	void ReSeed(u32 seed)
	{
		// Shift the seed a little bit around to fix small seeds.
		m_State = seed * 31337;
		if(m_State == 0)
			m_State = 0x3412353f; // This number is absolutly random.
	}

	//! Generate 32-Random bits.
	u32 GetBits() const
	{
		u32 x = m_State;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		m_State = x;

		return x;
	}

	//! Generate an integer
	/**
	\return A random integer between min and max inclusive
	*/
	inline int GetInt(int min, int max) const
	{
		if(min > max) {
			int t = min;
			min = max;
			max = t;
		}
		if(min == max)
			return min;

		const unsigned int range = max - min;
		const unsigned int maxRand = (0xFFFFFFFF / range)*range;

		unsigned int rand;
		do {
			rand = GetBits();
		} while(rand > maxRand);

		return min + (rand%range);
	}

	//! Generate a random boolean
	/**
	\param probability The probility for the value to be true, if the value is outside the range [0, 1] false is returned
	\return A random boolean
	*/
	inline bool GetBool(float probability = 0.5f) const
	{
		if(probability < 0.0f || probability > 1.0f)
			return false;

		if(GetBits() < (u32)(probability * 0xFFFFFFFF))
			return true;
		else
			return false;
	}

	//! Generate a random float
	/**
	\return A random floating point number between 0 and 1 inclusive
	*/
	inline float GetFloat() const
	{
		return (float)GetBits() / (float)0xFFFFFFFF;
	}

	//! Generate a random float
	/**
	\return A random float point number between min and max inclusive
	*/
	inline float GetFloat(float min, float max) const
	{
		return min + GetFloat() * (max-min);
	}

	//! Generate a random color
	/**
	\param alpha The alpha channel of the color, if alpha is smaller than 0 its generated random
	\return A random generated color
	*/
	inline video::Colorf GetColorf(float alpha = -1.0f) const
	{
		video::Colorf out(GetFloat(), GetFloat(), GetFloat(), alpha);
		if(alpha < 0.0f)
			out.a = GetFloat();

		return out;
	}

	//! Generate a random color
	/**
	\param alpha The alpha channel of the color, if alpha is smaller than 0 its generated random
	\return A random generated color
	*/
	inline video::Color GetColor(int alpha = -1) const
	{
		u32 out = GetBits();
		if(alpha >= 0) {
			out &= 0x00FFFFFF;
			out |= (alpha & 0x000000FF) << 24;
		}

		return out;
	}

	//! Generate a random floating point vector inside the unit square
	/**
	\return A random generated vector
	*/
	inline math::vector2f GetVector2() const
	{
		return math::vector2f(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f));
	}

	//! Generate a random vector inside a box
	/**
	\param min The min corner of the box
	\param max The max corner of the box
	\return A random vector
	*/
	inline math::vector2f GetVector2(const math::vector2f& min, const math::vector2f& max) const
	{
		return math::vector2f(
			GetFloat(min.x, max.x),
			GetFloat(min.y, max.y));
	}

	//! Generate a random vector inside a rect
	/**
	\param rect The rect in which the vector is placed
	\return A random vector
	*/
	inline math::vector2f GetVector2(const math::rectf& rect) const
	{
		return math::vector2f(
			GetFloat(rect.left, rect.right),
			GetFloat(rect.bottom, rect.top));
	}

	//! Generate a random vector inside the unit circle
	inline math::vector2f GetVector2Circle(float radius = 1.0f) const
	{
		const float baseLine = GetFloat(-1.0f, 1.0f);
		const float s = sqrt(1-baseLine*baseLine);
		const float height = GetFloat(-s, s);

		return radius * math::vector2f(baseLine, height);
	}

	//! Generate a random vector on the circle curve
	inline math::vector2f GetVector2CircleBorder(float radius = 1.0f) const
	{
		const auto angle = GetAngle();
		return math::vector2f::BuildFromPolar(angle, radius);
	}

	//! Generate a random vector inside the unit cube
	/**
	\return A random vector
	*/
	inline math::vector3f GetVector3() const
	{
		return math::vector3f(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f));
	}

	//! Generate a random vector inside a box
	/**
	\param min The min corner of the box
	\param max The max corner of the box
	\return A random vector
	*/
	inline math::vector3f GetVector3(const math::vector3f& min, const math::vector3f& max) const
	{
		return math::vector3f(
			GetFloat(min.x, max.x),
			GetFloat(min.y, max.y),
			GetFloat(min.z, max.z));
	}

	//! Generate random vector inside the sphere
	/**
	\param radius The radius of the sphere.
	\return A random point in the sphere.
	*/
	inline math::vector3f GetVector3Sphere(float radius=1.0f) const
	{
		math::vector3f v;
		do {
			v = GetVector3();
		} while(v.GetLengthSq() >= 1.0f);

		return v*radius;
	}

	//! Generate a random vector inside a box
	/**
	\param box The box in which the vector is placed
	\return A random vector
	*/
	inline math::vector3f GetVector3(const math::aabbox3df& box) const
	{
		return GetVector3(box.minCorner, box.maxCorner);
	}

	//! Generate a rangom angle between two bounds.
	/**
	\param min The lower bound
	\param max The upper bound
	\return A random angle between min and max.
	*/
	inline math::anglef GetAngle(math::anglef min, math::anglef max) const
	{
		return math::anglef::Radian(GetFloat(min.Radian(), max.Radian()));
	}

	//! Generate a random angle on the full circle
	/**
	\reutnr A random angle
	*/
	inline math::anglef GetAngle() const
	{
		return GetAngle(math::anglef::ZERO, math::anglef::FULL);
	}

private:
	mutable u32 m_State;
};

} // namspace core
} 


#endif