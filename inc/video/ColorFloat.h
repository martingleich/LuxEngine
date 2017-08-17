#ifndef INCLUDED_COLOR_FLOAT_H
#define INCLUDED_COLOR_FLOAT_H
#include "core/lxFormat.h"
#include "core/lxTypes.h"

namespace lux
{
namespace video
{
//! Floating-Point color
/**
Saves a three component color with alpha, in floating point format.
*/
class Colorf
{
public:
	float r; //!< The red color channel.
	float g; //!< The green color channel.
	float b; //!< The blue color channel.
	float a; //!< The alpha channel.

public:
	//! Default construct to transparent black.
	Colorf() :
		r(0.0f),
		g(0.0f),
		b(0.0f),
		a(0.0f)
	{
	}

	//! Construct from single values.
	/**
		Alpha is set to 1.
	*/
	Colorf(const float _r, const float _g, const float _b) : r(_r), g(_g), b(_b), a(1.0f)
	{
	}

	//! Construct from single values.
	Colorf(const float _r, const float _g, const float _b, const float _a) : r(_r), g(_g), b(_b), a(_a)
	{
	}

	//! Construct from color.
	Colorf(Color c)
	{
		SetInt((u32)(c.GetAlpha()), c.GetRed(), c.GetGreen(), c.GetBlue());
	}

	//! Construct from color(u32)
	Colorf(u32 c)
	{
		Color rc(c);
		*this = Colorf(rc);
	}

	static Colorf FromHex(u32 c)
	{
		return Colorf(c);
	}

	static Colorf FromRGB(float r, float g, float b, float a = 1.0f)
	{
		return Colorf(r, g, b, a);
	}

	static Colorf FromHSV(float h, float s, float v, float a = 1.0f)
	{
		Colorf out;
		HSVToRGB(h, s, v, out.r, out.g, out.b);
		out.a = a;
		return out;
	}

	u32 ToHex() const
	{
		return ((a >= 1.0f ? 255 : a <= 0.0f ? 0 : (u32)(a * 255.0f)) << 24) |
			((r >= 1.0f ? 255 : r <= 0.0f ? 0 : (u32)(r * 255.0f)) << 16) |
			((g >= 1.0f ? 255 : g <= 0.0f ? 0 : (u32)(g * 255.0f)) << 8) |
			((b >= 1.0f ? 255 : b <= 0.0f ? 0 : (u32)(b * 255.0f)) << 0);
	}

	void ToHSV(float& h, float& s, float& v) const
	{
		RGBToHSV(r, g, b, h, s, v);
	}

	//! Mix with other color
	Colorf& operator+=(const Colorf& c)
	{
		r += c.r;
		g += c.g;
		b += c.b;
		a += c.a;
		return *this;
	}

	Colorf& operator-=(const Colorf& c)
	{
		r -= c.r;
		g -= c.g;
		b -= c.b;
		a -= c.a;
		return *this;
	}
	//! Change colors intensity
	/**
		Alpha is changed too.
	*/
	Colorf& operator*=(const float f)
	{
		r *= f;
		g *= f;
		b *= f;
		a *= f;
		return *this;
	}

	//! Mix with other color.
	Colorf operator+(const Colorf& other) const
	{
		Colorf out(*this);
		out += other;
		return out;
	}

	Colorf operator-(const Colorf& other) const
	{
		Colorf out(*this);
		out -= other;
		return out;
	}

	//! Change colors intensity.
	/**
	Alpha is changed too.
	*/
	Colorf operator*(const float f) const
	{
		Colorf out(*this);
		out *= f;
		return out;
	}

	Colorf operator*(const Colorf& other) const
	{
		return Colorf(
			r*other.r,
			g*other.g,
			b*other.b,
			a*other.a);
	}

	Colorf& operator*=(const Colorf& other)
	{
		r *= other.r;
		g *= other.g;
		b *= other.b;
		a *= other.a;
		return *this;
	}

	//! Equality with other color.
	/**
		Is true, if the colors are the same, including alpha.
	*/
	bool operator==(const Colorf& other) const
	{
		return ((r == other.r) && (g == other.g) && (b == other.b) && (a == other.a));
	}

	//! Inquality with other color.
	/**
		Is false, if the colors are the same, including alpha.
	*/
	bool operator!=(const Colorf& other) const
	{
		return !(*this == other);
	}

	//! Comparison for sorting.
	bool operator<(const Colorf& other) const
	{
		if(r != other.r)
			return r < other.r;
		if(g != other.g)
			return g < other.g;
		if(b != other.b)
			return b < other.b;
		if(a != other.a)
			return a < other.a;
		return false;
	}

	//! Set the color from components.
	/**
	0 equals 0.0, and 255 equals 1.0.
	*/
	void SetInt(u32 _a, u32 _r,
		u32 _g, u32 _b)
	{
		const float factor = 0.0039215686274509803921568627451f;
		r = factor * (float)(_r);
		g = factor * (float)(_g);
		b = factor * (float)(_b);
		a = factor * (float)(_a);
	}

	//! Set the color from components.
	void Set(float _a, float _r, float _g, float _b)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	void SetRed(float _r) { r = _r; }
	void SetGreen(float _g) { g = _g; }
	void SetBlue(float _b) { b = _b; }
	void SetAlpha(float _a) { a = _a; }

	float GetRed() const { return r; }
	float GetBlue() const { return b; }
	float GetGreen() const { return g; }
	float GetAlpha() const { return a; }

	void Clamp()
	{
		r = math::Clamp<float>(r, 0, 1);
		g = math::Clamp<float>(g, 0, 1);
		b = math::Clamp<float>(b, 0, 1);
	}

	//! Get the negated color
	inline Colorf GetNegative() const
	{
		return Colorf(1.0f - r, 1.0f - g, 1.0f - g, 1.0f - a);
	}

	//! Get the brightness of the color.
	/**
	The brightness lies betwen 0 for pure black, and 1 for pure white.
	\return The Brightness of the color.
	*/
	inline float GetLuminance() const
	{
		return r * 0.299f + g * 0.587f + b * 0.114f;
	}

	//! Has the color an alpha value smaller than 1
	inline bool HasAlpha() const
	{
		return a < 1.0f;
	}

	inline Colorf WeightWith(const Colorf& c) const
	{
		return Colorf(r*c.r, g*c.g, b*c.b, a*c.a);
	}
};

inline Colorf operator*(const float f, const Colorf& a)
{
	Colorf out(a); out *= f; return out;
}

inline void conv_data(format::Context& ctx, Colorf format, format::Placeholder& placeholder)
{
	using namespace format;
	placeholder.type = 'a';

	ConvertAddString(ctx, format::StringType::Ascii, "[r=", 3);
	conv_data(ctx, format.r, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " g=", 3);
	conv_data(ctx, format.g, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " b=", 3);
	conv_data(ctx, format.b, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " a=", 3);
	conv_data(ctx, format.a, placeholder);

	ConvertAddString(ctx, format::StringType::Ascii, "]", 1);
}

}

namespace core
{
namespace Types
{
LUX_API Type Colorf();
}
template<> inline Type GetTypeInfo<video::Colorf>() { return Types::Colorf(); }
} // namespace core

}

#endif // #ifndef INCLUDED_COLOR_FLOAT_H