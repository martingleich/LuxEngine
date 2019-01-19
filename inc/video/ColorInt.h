#ifndef INCLUDED_LUX_COLOR_INT_H
#define INCLUDED_LUX_COLOR_INT_H
#include "core/lxFormat.h"
#include "math/lxMath.h"
#include "core/lxTypes.h"

namespace lux
{
namespace video
{

//! Real 32-Bit color
/**
Saves a color with integral 8 bit red,green,blue and alpha channels.
*/
class Color
{
public:
	//! Construct transparent black.
	Color() : c(0)
	{
	}

	static Color Uniform(u32 f, u32 alpha = 255)
	{
		return Color(f, f, f, alpha);
	}
	static Color Faded(Color c, u32 a)
	{
		return c.SetAlpha(a);
	}

	//! Construct from other color.
	Color(const Color& _c) : c(_c.c)
	{
	}

	//! Construct from channels whihout alpha
	Color(u32 r, u32 g, u32 b)
	{
		c = (u32)((0xFF << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF));
	}

	//! Construct from channels with alpha
	Color(u32 r, u32 g, u32 b, u32 a)
	{
		c = (u32)(((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF));
	}

	//! Construct from a value in A8R8G8B8 format.
	Color(u32 _c) : c(_c)
	{
	}

	//! The value of the color a bits
	/**
	\return The color in A8R8G8B8 format
	*/
	u32 ToDWORD() const
	{
		return c;
	}

	//! Return Alpha
	u32 GetAlpha()  const
	{
		return c >> 24;
	}
	//! Return Red
	u32 GetRed()    const
	{
		return (c >> 16) & 0xFF;
	}
	//! Return Green
	u32 GetGreen()  const
	{
		return (c >> 8) & 0xFF;
	}
	//! Return Blue
	u32 GetBlue()   const
	{
		return c & 0xFF;
	}

	//! Set Alpha
	u32 SetAlpha(u32 a)
	{
		return c = ((a & 0xFF) << 24) | (c & 0x00FFFFFF);
	}

	//! Set Red
	u32 SetRed(u32 r)
	{
		return c = ((r & 0xFF) << 16) | (c & 0xFF00FFFF);
	}
	//! Set Green
	u32 SetGreen(u32 g)
	{
		return c = ((g & 0xFF) << 8) | (c & 0xFFFF00FF);
	}
	//! Set Blue
	u32 SetBlue(u32 b)
	{
		return c = (b & 0xFF) | (c & 0xFFFFFF00);
	}

	//! Has the color a alpha channel != 255.
	bool HasAlpha() const
	{
		return (GetAlpha() != 0xFF);
	}

	//! Assignment
	Color& operator=(Color other)
	{
		c = other.c;  return *this;
	}

	//! Add another color(per channel)
	Color& operator+= (Color other)
	{
		*this = Color(math::Min(GetRed() + other.GetRed(), 255u),
			math::Min(GetGreen() + other.GetGreen(), 255u),
			math::Min(GetBlue() + other.GetBlue(), 255u),
			math::Min(GetAlpha() + other.GetAlpha(), 255u));
		return *this;
	}

	//! Change Intensity
	Color& operator*=(float f)
	{
		*this = Color(math::Min((u32)(GetRed() * f), 255u),
			math::Min((u32)(GetGreen() * f), 255u),
			math::Min((u32)(GetBlue() * f), 255u),
			math::Min((u32)(GetAlpha() * f), 255u));
		return *this;
	}

	Color operator*(Color other) const
	{
		return Color(
			(GetRed() * other.GetRed()) / 255,
			(GetGreen() * other.GetGreen()) / 255,
			(GetBlue() * other.GetBlue()) / 255,
			(GetAlpha() * other.GetAlpha()) / 255);
	}

	Color& operator*=(Color other)
	{
		*this = Color(
			(GetRed() * other.GetRed()) / 255,
			(GetGreen() * other.GetGreen()) / 255,
			(GetBlue() * other.GetBlue()) / 255,
			(GetAlpha() * other.GetAlpha()) / 255);
		return *this;
	}

	//! Addition(Mixing colors)
	Color operator+(Color other) const
	{
		Color out(*this);
		out += other;
		return out;
	}

	//! scale color(Making it brighter or darker)
	Color operator*(float f) const
	{
		Color out(*this);
		out *= f;
		return out;
	}

	//! Equality
	bool operator==(Color other) const
	{
		return c == other.c;
	}

	//! Inequality
	bool operator!=(Color other) const
	{
		return c != other.c;
	}

	//! Comparison operator for sorting.
	bool operator<(Color other) const
	{
		return c < other.c;
	}

	//! Set from channels(Byte)
	void Set(u8 _a, u8 _r,
		u8 _g, u8 _b)
	{
		*this = Color(_r, _g, _b, _a);
	}

	//! Set from channels(Float)
	void SetF(float a, float r, float g, float b)
	{
		a = math::Clamp(a, 0.0f, 1.0f);
		r = math::Clamp(r, 0.0f, 1.0f);
		g = math::Clamp(g, 0.0f, 1.0f);
		b = math::Clamp(b, 0.0f, 1.0f);

		*this = Color((u32)(r * 255), (u32)(g * 255), (u32)(b * 255), (u32)(a * 255));
	}

	//! Return the luminance of the color
	inline float GetLuminance()
	{
		return GetRed() * 0.299f + GetGreen() * 0.587f + GetBlue() * 0.114f;
	}

	//! Get the inverse of the color
	/**
	Remark: The alpha channels isn't inverted.
	*/
	inline Color GetNegative()
	{
		u32 o = 0xFFFFFFFF - c;
		o &= 0x00FFFFFF;
		o |= GetAlpha() << 24;
		return o;
	}

	Color Scaled(float f) const
	{
		Color out(*this);
		out *= f;
		out.SetAlpha(GetAlpha());
		return out;
	}

	Color ScaledA(float f) const
	{
		Color out(*this);
		out *= f;
		return out;
	}

	Color GetInverted() const
	{
		return Color(255-GetRed(), 255-GetGreen(), 255-GetBlue(), GetAlpha());
	}
	Color GetInvertedA() const
	{
		return Color(255-GetRed(), 255-GetGreen(), 255-GetBlue(), 255-GetAlpha());
	}

	//! Predefined Colors(Alpha = 255)
	enum EPredefinedColors : u32
	{
#include "InternalColorList.h"
	};

private:
	u32 c;
};

inline Color operator*(const float f, Color a)
{
	Color out(a); out *= f; return out;
}

inline void fmtPrint(format::Context& ctx, Color format, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	format::vformat(ctx, "{!h}", format.ToDWORD());
}

} // namespace video

namespace core
{
namespace Types
{
LUX_API Type Color();
}
template<> struct TemplType<video::Color> { static Type Get() { return Types::Color(); } };
} // namespace core

namespace math
{
template <>
inline video::Color Lerp(const video::Color& a, const video::Color& b, float t)
{
	t = math::Clamp<float>(t, 0, 1);

	s32 aRed = a.GetRed();
	s32 bRed = b.GetRed();
	s32 aGreen = a.GetGreen();
	s32 bGreen = b.GetGreen();
	s32 aBlue = a.GetBlue();
	s32 bBlue = b.GetBlue();
	s32 aAlpha = a.GetAlpha();
	s32 bAlpha = b.GetAlpha();

	u32 red = (u32)(aRed + (s32)(t*(bRed - aRed)));
	u32 green = (u32)(aGreen + (s32)(t*(bGreen - aGreen)));
	u32 blue = (u32)(aBlue + (s32)(t*(bBlue - aBlue)));
	u32 alpha = (u32)(aAlpha + (s32)(t*(bAlpha - aAlpha)));

	return video::Color(red, green, blue, alpha);
}

} // namespace math
} // namespace lux

#endif // #ifndef INCLUDED_LUX_COLOR_INT_H