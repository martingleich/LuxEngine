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
class ColorF
{
public:
	float r; //!< The red color channel.
	float g; //!< The green color channel.
	float b; //!< The blue color channel.
	float a; //!< The alpha channel.

public:
	//! Default construct to transparent black.
	ColorF() :
		r(0.0f),
		g(0.0f),
		b(0.0f),
		a(0.0f)
	{
	}

	static ColorF Uniform(float f, float alpha = 1.0f)
	{
		return ColorF(f,f,f,alpha);
	}

	//! Construct from single values.
	/**
		Alpha is set to 1.
	*/
	ColorF(const float _r, const float _g, const float _b) : r(_r), g(_g), b(_b), a(1.0f)
	{
	}

	//! Construct from single values.
	ColorF(const float _r, const float _g, const float _b, const float _a) : r(_r), g(_g), b(_b), a(_a)
	{
	}

	//! Construct from color.
	ColorF(Color c)
	{
		SetInt((u32)(c.GetAlpha()), c.GetRed(), c.GetGreen(), c.GetBlue());
	}

	//! Construct from color(u32)
	ColorF(u32 c)
	{
		Color rc(c);
		*this = ColorF(rc);
	}

	static ColorF FromHex(u32 c)
	{
		return ColorF(c);
	}

	static ColorF FromRGB(float r, float g, float b, float a = 1.0f)
	{
		return ColorF(r, g, b, a);
	}

	static ColorF FromHSV(float h, float s, float v, float a = 1.0f)
	{
		ColorF out;
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
	ColorF& operator+=(const ColorF& c)
	{
		r += c.r;
		g += c.g;
		b += c.b;
		a += c.a;
		return *this;
	}

	ColorF& operator-=(const ColorF& c)
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
	ColorF& operator*=(const float f)
	{
		r *= f;
		g *= f;
		b *= f;
		a *= f;
		return *this;
	}

	//! Mix with other color.
	ColorF operator+(const ColorF& other) const
	{
		ColorF out(*this);
		out += other;
		return out;
	}

	ColorF operator-(const ColorF& other) const
	{
		ColorF out(*this);
		out -= other;
		return out;
	}

	//! Change colors intensity.
	/**
	Alpha is changed too.
	*/
	ColorF operator*(const float f) const
	{
		ColorF out(*this);
		out *= f;
		return out;
	}

	ColorF operator*(const ColorF& other) const
	{
		return ColorF(
			r*other.r,
			g*other.g,
			b*other.b,
			a*other.a);
	}

	ColorF& operator*=(const ColorF& other)
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
	bool operator==(const ColorF& other) const
	{
		return ((r == other.r) && (g == other.g) && (b == other.b) && (a == other.a));
	}

	//! Inquality with other color.
	/**
		Is false, if the colors are the same, including alpha.
	*/
	bool operator!=(const ColorF& other) const
	{
		return !(*this == other);
	}

	//! Comparison for sorting.
	bool operator<(const ColorF& other) const
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
	inline ColorF GetNegative() const
	{
		return ColorF(1.0f - r, 1.0f - g, 1.0f - g, 1.0f - a);
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

	inline ColorF WeightWith(const ColorF& c) const
	{
		return ColorF(r*c.r, g*c.g, b*c.b, a*c.a);
	}

	float* Data()
	{
		return &r;
	}
	const float* Data() const
	{
		return &r;
	}
};

inline ColorF operator*(const float f, const ColorF& a)
{
	ColorF out(a); out *= f; return out;
}

inline void fmtPrint(format::Context& ctx, ColorF format, format::Placeholder& placeholder)
{
	using namespace format;
	placeholder.type = 'a';

	ctx.AddSlice(3, "[r=");
	fmtPrint(ctx, format.r, placeholder);
	ctx.AddSlice(3, " g=");
	fmtPrint(ctx, format.g, placeholder);
	ctx.AddSlice(3, " b=");
	fmtPrint(ctx, format.b, placeholder);
	ctx.AddSlice(3, " a=");
	fmtPrint(ctx, format.a, placeholder);
	ctx.AddSlice(1, "]");
}

}

namespace core
{
namespace Types
{
LUX_API Type ColorF();
}
template<> struct TemplType<video::ColorF> { static Type Get() { return Types::ColorF(); } };
} // namespace core

}

#endif // #ifndef INCLUDED_COLOR_FLOAT_H