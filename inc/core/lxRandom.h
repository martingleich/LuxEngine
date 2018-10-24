#ifndef INCLUDED_LUX_RANDOM_H
#define INCLUDED_LUX_RANDOM_H
#include "core/Clock.h"

#include "video/Color.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Angle.h"
#include "math/AABBox.h"
#include "math/Rect.h"

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

	//! Seed the random number generator, with the current time
	void ReSeed()
	{
		auto ticks = core::Clock::GetTicks().Count();
		m_State = (u32)core::HashSequence(&ticks, sizeof(ticks));
		if(m_State == 0)
			m_State = 0x3412353f; // This number is absolutly random.
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
		if(probability <= 0.0f || probability > 1.0f)
			return false;

		float max = probability * 0xFFFFFFFF;
		if((float)GetBits() <= max)
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
		return min + GetFloat() * (max - min);
	}

	//! Generate a random color
	/**
	\param alpha The alpha channel of the color, if alpha is smaller than 0 its generated random
	\return A random generated color
	*/
	inline video::ColorF GetColorf(float alpha = -1.0f) const
	{
		video::ColorF out(GetFloat(), GetFloat(), GetFloat(), alpha);
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
	inline math::Vector2F GetVector2() const
	{
		return math::Vector2F(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f));
	}

	//! Generate a random vector inside a box
	/**
	\param min The min corner of the box
	\param max The max corner of the box
	\return A random vector
	*/
	inline math::Vector2F GetVector2(const math::Vector2F& min, const math::Vector2F& max) const
	{
		return math::Vector2F(
			GetFloat(min.x, max.x),
			GetFloat(min.y, max.y));
	}

	//! Generate a random vector inside a rect
	/**
	\param rect The rect in which the vector is placed
	\return A random vector
	*/
	inline math::Vector2F GetVector2(const math::RectF& rect) const
	{
		return math::Vector2F(
			GetFloat(rect.left, rect.right),
			GetFloat(rect.bottom, rect.top));
	}

	//! Generate a random vector inside the unit circle
	inline math::Vector2F GetVector2Circle(float radius = 1.0f) const
	{
		const float baseLine = GetFloat(-1.0f, 1.0f);
		const float s = std::sqrt(1 - baseLine*baseLine);
		const float height = GetFloat(-s, s);

		return radius * math::Vector2F(baseLine, height);
	}

	//! Generate a random vector on the circle curve
	inline math::Vector2F GetVector2CircleBorder(float radius = 1.0f) const
	{
		const auto angle = GetAngle();
		return math::Vector2F::BuildFromPolar(angle, radius);
	}

	//! Generate a random vector inside the unit cube
	/**
	\return A random vector
	*/
	inline math::Vector3F GetVector3() const
	{
		return math::Vector3F(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f));
	}

	//! Generate a random vector inside a box
	/**
	\param min The min corner of the box
	\param max The max corner of the box
	\return A random vector
	*/
	inline math::Vector3F GetVector3(const math::Vector3F& min, const math::Vector3F& max) const
	{
		return math::Vector3F(
			GetFloat(min.x, max.x),
			GetFloat(min.y, max.y),
			GetFloat(min.z, max.z));
	}

	//! Generate random vector inside the sphere
	/**
	\param radius The radius of the sphere.
	\return A random point in the sphere.
	*/
	inline math::Vector3F GetVector3Sphere(float radius = 1.0f) const
	{
		math::Vector3F v;
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
	inline math::Vector3F GetVector3(const math::AABBoxF& box) const
	{
		return GetVector3(box.minCorner, box.maxCorner);
	}

	//! Generate a rangom angle between two bounds.
	/**
	\param min The lower bound
	\param max The upper bound
	\return A random angle between min and max.
	*/
	inline math::AngleF GetAngle(math::AngleF min, math::AngleF max) const
	{
		return math::AngleF::Radian(GetFloat(min.Radian(), max.Radian()));
	}

	//! Generate a random angle on the full circle
	/**
	\reutnr A random angle
	*/
	inline math::AngleF GetAngle() const
	{
		return GetAngle(math::AngleF::ZERO, math::AngleF::FULL);
	}

private:
	mutable u32 m_State;
};

class Distribution
{
	enum class EDistribution
	{
		Uniform,
	};
public:
	Distribution() :
		distribution(EDistribution::Uniform),
		min(0),
		max(0)
	{
	}
	Distribution(float f)
	{
		*this = Fixed(f);
	}
	static Distribution Fixed(float life)
	{
		Distribution time;
		time.distribution = EDistribution::Uniform;
		time.min = time.max = life;
		return time;
	}
	static Distribution Uniform(float min, float max)
	{
		Distribution time;
		time.distribution = EDistribution::Uniform;
		time.min = min;
		time.max = max;
		return time;
	}
	static Distribution Infinite()
	{
		Distribution time;
		time.distribution = EDistribution::Uniform;
		time.min = INFINITY;
		time.max = INFINITY;
		return time;
	}

	bool IsInfinite() const
	{
		return (min == INFINITY);
	}
	float GetAverage() const
	{
		return (min + max) / 2;
	}

	float GetPercentile(float p)
	{
		switch(distribution) {
		case EDistribution::Uniform:
			if(min == max)
				return min;
			return (1 - p) * min + p * max;
		default:
			return 0;
		}
	}

	float Sample(core::Randomizer& randomizer) const
	{
		switch(distribution) {
		case EDistribution::Uniform:
			if(min == max)
				return min;
			return randomizer.GetFloat(min, max);
		default:
			return 0;
		}
	}

	Distribution& operator*=(float f)
	{
		switch(distribution) {
		case EDistribution::Uniform:
			min *= f;
			max *= f;
			break;
		}

		return *this;
	}
	Distribution& operator+=(float f)
	{
		switch(distribution) {
		case EDistribution::Uniform:
			min += f;
			max += f;
			break;
		}

		return *this;
	}

	Distribution& operator-=(float f)
	{
		return (*this += -f);
	}

	Distribution& operator/=(float f)
	{
		return (*this *= (1 / f));
	}

private:
	EDistribution distribution;
	float min;
	float max;
};

inline Distribution operator+(const Distribution& a, float f)
{
	Distribution out(a);
	out += f;
	return out;
}

inline Distribution operator+(float f, const Distribution& a)
{
	Distribution out(a);
	out += f;
	return out;
}

inline Distribution operator*(const Distribution& a, float f)
{
	Distribution out(a);
	out *= f;
	return out;
}

inline Distribution operator/(const Distribution& a, float f)
{
	Distribution out(a);
	out /= f;
	return out;
}

inline Distribution operator*(float f, const Distribution& a)
{
	Distribution out(a);
	out *= f;
	return out;
}

inline Distribution operator-(const Distribution& a, float f)
{
	Distribution out(a);
	out -= f;
	return out;
}

inline Distribution operator-(float f, const Distribution& a)
{
	Distribution out;
	out = -1 * (a - f);
	return out;
}

inline Distribution operator-(const Distribution& a)
{
	Distribution out(a);
	out *= -1;
	return out;
}

} // namespace core
} // namespace lux

#endif
