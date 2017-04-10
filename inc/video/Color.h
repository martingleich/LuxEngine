#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H
#include "math/lxMath.h"
#include "core/lxFormat.h"

namespace lux
{
namespace video
{

//! A color format
/**
Contains information about a format, and can convert between formate.
*/
class ColorFormat
{
public:
	//! The available color formats
	enum EColorFormat : u32
	{
		R8G8B8 = 0,    // 24 Bit
		A8R8G8B8,    // 32 Bit
		A1R5G5B5,    // 16 Bit
		R5G6B5,        // 16 Bit

		X8,
		X16,

		// Floating Point Formats
		R16F,
		G16R16F,
		A16B16G16R16F,
		R32F,
		G32R32F,
		A32B32G32R32F,

		UNKNOWN,
	};

	//! The number of available formats
	static const u32 FORMAT_COUNT = 12;

public:
	//! Constructor from format
	ColorFormat(EColorFormat f = UNKNOWN) :
		m_Format(f)
	{
	}

	//! Assign from format
	ColorFormat& operator=(EColorFormat f)
	{
		m_Format = f;
		return *this;
	}

	//! Convert the format to an id
	explicit operator u32() const
	{
		return m_Format;
	}

	bool operator==(ColorFormat other) const
	{
		return m_Format == other.m_Format;
	}

	bool operator!=(ColorFormat other) const
	{
		return m_Format != other.m_Format;
	}

	//! Smaller operator for sorting.
	bool operator<(ColorFormat other) const
	{
		return (u32)m_Format < (u32)other.m_Format;
	}

	//! Get the string repersenting this colorformat
	const char* AsString() const
	{
		static const char* STRINGS[] = {
			"R8G8B8",
			"A8R8G8B8",
			"A1R5G5B5",
			"R5G6B5",
			"X8",
			"X16",
			"R16F",
			"G16R16F",
			"A16B16G16R16F",
			"R32F",
			"G32R32F",
			"A32B32G32R32F",
			"UNKNOWN"
		};

		return STRINGS[m_Format];
	}

	//! The number of bits in a pixel for this format
	inline u8 GetBitsPerPixel() const
	{
		static const u8 BitPerPixel[FORMAT_COUNT] = {24, 32, 16, 16, 8, 16, 16, 32, 64, 32, 64, 128};
		if(m_Format >= FORMAT_COUNT)
			return 0;

		return BitPerPixel[(u32)m_Format];
	}

	//! The number of bytes in a pixel for this format
	inline u32 GetBytePerPixel() const
	{
		return GetBitsPerPixel() / 8;
	}

	//! The mask for the red channel
	/**
	Binary anding a pixel with this value keeps only the red pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetRedMask() const
	{
		static const u32 Mask[6] = {0xff0000, 0xff0000, 0x7c00, 0xf800, 0xff, 0xffff};
		if(IsFloatingPoint())
			return 0;
		return Mask[(u32)m_Format];
	}

	//! The mask for the green channel
	/**
	Binary anding a pixel with this value keeps only the green pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetGreenMask() const
	{
		static const u32 Mask[6] = {0xff00, 0xff00, 0x3e0, 0x7e0, 0xff, 0xffff};
		if(IsFloatingPoint())
			return 0;
		return Mask[(u32)m_Format];
	}

	//! The mask for the blue channel
	/**
	Binary anding a pixel with this value keeps only the blue pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetBlueMask() const
	{
		static const u32 Mask[6] = {0xff, 0xff, 0x1f, 0x1f, 0xff, 0xffff};
		if(IsFloatingPoint())
			return 0;
		return Mask[(u32)m_Format];
	}

	//! The mask for the alpha channel
	/**
	Binary anding a pixel with this value keeps only the alpha pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetAlphaMask() const
	{
		static const u32 Mask[6] = {0x0, 0xff000000, 0x8000, 0x0, 0x0, 0x0};
		if(IsFloatingPoint())
			return 0;
		return Mask[(u32)m_Format];
	}

	//! Can this format contain a alpha value
	inline bool HasAlpha() const
	{
		if(m_Format == A16B16G16R16F ||
			m_Format == A1R5G5B5 ||
			m_Format == A32B32G32R32F ||
			m_Format == A8R8G8B8)
			return true;
		else
			return false;
	}

	//! Is this format a floating point value
	inline bool IsFloatingPoint() const
	{
		if(m_Format >= R16F && m_Format <= UNKNOWN)
			return true;
		else
			return false;
	}

	// TODO: Add floating-point conversions.
	//! Convert a color in this format to A8R8G8B8
	/**
	\param in A pointer to a pixel in this format
	\return The passed pixel in format A8R8G8B8
	*/
	inline u32 FormatToA8R8G8B8(const u8* in) const
	{
		u32 out;
		u32 in16;

		switch(m_Format) {
		case R8G8B8:
			out = 0xFF000000 | (in[0] << 16) | (in[1] << 8) | (in[2] << 0);
			break;
		case A8R8G8B8:
			out = *((u32*)(in));
			break;
		case A1R5G5B5:
			in16 = *((u16*)in);
			out = (((u32)in16 & 0x8000) >> 31) & 0xFF00 |
				((in16 & 0x7C00) << 9) | ((in16 & 0x7000) << 4) |
				((in16 & 0x03E0) << 6) | ((in16 & 0x0380) << 1) |
				((in16 & 0x001F) << 3) | ((in16 & 0x001C) >> 2);
			break;
		case R5G6B5:
			in16 = *((u16*)in);
			out =
				((in16 & 0xF800) << 8) |
				((in16 & 0x07E0) << 5) |
				((in16 & 0x001F) << 3);
			break;
		case X8:
			out = 0xFF000000 | (*in << 16) | (*in << 8) << (*in << 0);
			break;
		case X16:
			out = 0xFF000000 | (in[1] << 16) | (in[1] << 8) << (in[1] << 0);
			break;
		default:
			assertNeverReach("Colorformat not implemented.");
			out = 0;
		}

		return out;
	}

	//! Convert a pixel from A8R8G8B8 to this format
	/**
	\param in A pixel in A8R8G8B8 format.
	\param [out] out Here the converted pixel is written.
	*/
	inline void A8R8G8B8ToFormat(u32 in, u8* out) const
	{
		switch(m_Format) {
		case R8G8B8:
			*(out + 1) = (u8)((in & 0x0000FF00) >> 8);
			*(out + 0) = (u8)((in & 0x00FF0000) >> 16);
			*(out + 2) = (u8)(in & 0x000000FF);
			break;
		case A8R8G8B8:
			*(u32*)out = in;
			break;
		case X8:
			*out = (((in >> 16) & 0xFF) * 76 + ((in >> 8) & 0xFF) * 150 + (in & 0xFF) * 29) / 255;
			break;
		case X16:
			*(u16*)out = static_cast<u16>((((in >> 16) & 0xFF) * 19595 + ((in >> 8) & 0xFF) * 38469 + (in & 0xFF) * 7471) / 255);
			break;
		case A1R5G5B5:
			*(u16*)out = static_cast<u16>(
				((in & 0x80000000) >> 16) |
				((in & 0x00F80000) >> 9) |
				((in & 0x0000F800) >> 6) |
				((in & 0x000000F8) >> 3));
			break;
		case R5G6B5:
			*(u16*)out = static_cast<u16>(
				((in & 0x00F80000) >> 8) |
				((in & 0x0000FC00) >> 5) |
				((in & 0x000000F8) >> 3));
			break;
		default:
			assertNeverReach("Colorformat not implemented.");
			memset(out, 0, GetBytePerPixel());
		}
	}

private:
	EColorFormat m_Format;
};

class Colorf;

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

	//! Construct from other color.
	Color(const Color& _c) : c(_c.c)
	{
	}

	inline Color(const Colorf& _c);

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
	explicit operator u32() const
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
	Color& operator *= (const float f)
	{
		*this = Color(math::Min((u32)(GetRed() * f), 255u),
			math::Min((u32)(GetGreen() * f), 255u),
			math::Min((u32)(GetBlue() * f), 255u),
			math::Min((u32)(GetAlpha() * f), 255u));
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
	void Set(float a, float r, float g, float b)
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

	//! Predefined Colors(Alpha = 255)
	enum EPredefinedColors : u32
	{
		Invisible = 0,
		AliceBlue = 0xFFF0F8FF,
		AntiqueWhite = 0xFFFAEBD7,
		Aqua = 0xFF00FFFF,
		Aquamarine = 0xFF7FFFD4,
		Azure = 0xFFF0FFFF,
		Beige = 0xFFF5F5DC,
		Bisque = 0xFFFFE4C4,
		Black = 0xFF000000,
		BlanchedAlmond = 0xFFFFEBCD,
		Blue = 0xFF0000FF,
		BlueViolet = 0xFF8A2BE2,
		Brown = 0xFFA52A2A,
		BurlyWood = 0xFFDEB887,
		CadetBlue = 0xFF5F9EA0,
		Chartreuse = 0xFF7FFF00,
		Chocolate = 0xFFD2691E,
		Coral = 0xFFFF7F50,
		CornflowerBlue = 0xFF6495ED,
		Cornsilk = 0xFFFFF8DC,
		Crimson = 0xFFDC143C,
		Cyan = 0xFF00FFFF,
		DarkBlue = 0xFF00008B,
		DarkCyan = 0xFF008B8B,
		DarkGoldenrod = 0xFFB8860B,
		DarkGray = 0xFFA9A9A9,
		DarkGreen = 0xFF006400,
		DarkKhaki = 0xFFBDB76B,
		DarkMagenta = 0xFF8B008B,
		DarkOliveGreen = 0xFF556B2F,
		DarkOrange = 0xFFFF8C00,
		DarkOrchid = 0xFF9932CC,
		DarkRed = 0xFF8B0000,
		DarkSalmon = 0xFFE9967A,
		DarkSeaGreen = 0xFF8FBC8F,
		DarkSlateBlue = 0xFF483D8B,
		DarkSlateGray = 0xFF2F4F4F,
		DarkTurquoise = 0xFF00CED1,
		DarkViolet = 0xFF9400D3,
		DeepPink = 0xFFFF1493,
		DeepSkyBlue = 0xFF00BFFF,
		DimGray = 0xFF696969,
		DodgerBlue = 0xFF1E90FF,
		Firebrick = 0xFFB22222,
		FloralWhite = 0xFFFFFAF0,
		ForestGreen = 0xFF228B22,
		Fuchsia = 0xFFFF00FF,
		Gainsboro = 0xFFDCDCDC,
		GhostWhite = 0xFFF8F8FF,
		Gold = 0xFFFFD700,
		Goldenrod = 0xFFDAA520,
		Gray = 0xFF808080,
		Green = 0xFF00FF00,
		GreenYellow = 0xFFADFF2F,
		Honeydew = 0xFFF0FFF0,
		HotPink = 0xFFFF69B4,
		IndianRed = 0xFFCD5C5C,
		Indigo = 0xFF4B0082,
		Ivory = 0xFFFFFFF0,
		Khaki = 0xFFF0E68C,
		Lavender = 0xFFE6E6FA,
		LavenderBlush = 0xFFFFF0F5,
		LawnGreen = 0xFF7CFC00,
		LemonChiffon = 0xFFFFFACD,
		LightBlue = 0xFFADD8E6,
		LightCoral = 0xFFF08080,
		LightCyan = 0xFFE0FFFF,
		LightGoldenrodYellow = 0xFFFAFAD2,
		LightGreen = 0xFF90EE90,
		LightGray = 0xFFD3D3D3,
		LightPink = 0xFFFFB6C1,
		LightSalmon = 0xFFFFA07A,
		LightSeaGreen = 0xFF20B2AA,
		LightSkyBlue = 0xFF87CEFA,
		LightSlateGray = 0xFF778899,
		LightSteelBlue = 0xFFB0C4DE,
		LightYellow = 0xFFFFFFE0,
		Lime = 0xFF00FF00,
		LimeGreen = 0xFF32CD32,
		Linen = 0xFFFAF0E6,
		Magenta = 0xFFFF00FF,
		Maroon = 0xFF800000,
		MediumAquamarine = 0xFF66CDAA,
		MediumBlue = 0xFF0000CD,
		MediumOrchid = 0xFFBA55D3,
		MediumPurple = 0xFF9370DB,
		MediumSeaGreen = 0xFF3CB371,
		MediumSlateBlue = 0xFF7B68EE,
		MediumSpringGreen = 0xFF00FA9A,
		MediumTurquoise = 0xFF48D1CC,
		MediumVioletRed = 0xFFC71585,
		MidnightBlue = 0xFF191970,
		MintCream = 0xFFF5FFFA,
		MistyRose = 0xFFFFE4E1,
		Moccasin = 0xFFFFE4B5,
		NavajoWhite = 0xFFFFDEAD,
		Navy = 0xFF000080,
		OldLace = 0xFFFDF5E6,
		Olive = 0xFF808000,
		OliveDrab = 0xFF6B8E23,
		Orange = 0xFFFFA500,
		OrangeRed = 0xFFFF4500,
		Orchid = 0xFFDA70D6,
		PaleGoldenrod = 0xFFEEE8AA,
		PaleGreen = 0xFF98FB98,
		PaleTurquoise = 0xFFAFEEEE,
		PaleVioletRed = 0xFFDB7093,
		PapayaWhip = 0xFFFFEFD5,
		PeachPuff = 0xFFFFDAB9,
		Peru = 0xFFCD853F,
		Pink = 0xFFFFC0CB,
		Plum = 0xFFDDA0DD,
		PowderBlue = 0xFFB0E0E6,
		Purple = 0xFF800080,
		Red = 0xFFFF0000,
		RosyBrown = 0xFFBC8F8F,
		RoyalBlue = 0xFF4169E1,
		SaddleBrown = 0xFF8B4513,
		Salmon = 0xFFFA8072,
		SandyBrown = 0xFFF4A460,
		SeaGreen = 0xFF2E8B57,
		SeaShell = 0xFFFFF5EE,
		Sienna = 0xFFA0522D,
		Silver = 0xFFC0C0C0,
		SkyBlue = 0xFF87CEEB,
		SlateBlue = 0xFF6A5ACD,
		SlateGray = 0xFF708090,
		Snow = 0xFFFFFAFA,
		SpringGreen = 0xFF00FF7F,
		SteelBlue = 0xFF4682B4,
		Tan = 0xFFD2B48C,
		Teal = 0xFF008080,
		Thistle = 0xFFD8BFD8,
		Tomato = 0xFFFF6347,
		Turquoise = 0xFF40E0D0,
		Violet = 0xFFEE82EE,
		Wheat = 0xFFF5DEB3,
		White = 0xFFFFFFFF,
		WhiteSmoke = 0xFFF5F5F5,
		Yellow = 0xFFFFFF00,
		YellowGreen = 0xFF9ACD32,
	};

private:
	u32 c;
};

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

	//! Copyconstructor
	Colorf(const Colorf& c) : a(c.a), r(c.r), g(c.g), b(c.b)
	{
	}

	//! Construct from single values.
	/**
		Alpha is set to 1.
	*/
	Colorf(const float _r, const float _g, const float _b) : a(1.0f), r(_r), g(_g), b(_b)
	{
	}

	//! Construct from single values.
	Colorf(const float _r, const float _g, const float _b, const float _a) : a(_a), r(_r), g(_g), b(_b)
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

	//! Cast to A8R8G8B8 color
	explicit operator u32() const
	{
		return ((a >= 1.0f ? 255 : a <= 0.0f ? 0 : (u32)(a * 255.0f)) << 24) |
			((r >= 1.0f ? 255 : r <= 0.0f ? 0 : (u32)(r * 255.0f)) << 16) |
			((g >= 1.0f ? 255 : g <= 0.0f ? 0 : (u32)(g * 255.0f)) << 8) |
			((b >= 1.0f ? 255 : b <= 0.0f ? 0 : (u32)(b * 255.0f)) << 0);
	}

	//! Assign other color
	Colorf& operator=(const Colorf& c)
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
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
};

inline Colorf operator*(const float f, const Colorf& a)
{
	Colorf out(a); out *= f; return out;
}
inline Color operator*(const float f, Color a)
{
	Color out(a); out *= f; return out;
}

inline Color::Color(const Colorf& _c)
{
	SetRed((u32)(_c.r * 255));
	SetBlue((u32)(_c.b*255));
	SetGreen((u32)(_c.g*255));
	SetAlpha((u32)(_c.a*255));
}

inline void conv_data(format::Context& ctx, ColorFormat format, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	format::ConvertAddString(ctx, format::StringType::Ascii, format.AsString(), strlen(format.AsString()));
}


}    // video

template<> inline core::Type core::GetTypeInfo<video::Colorf>() { return core::Type::ColorF; }
template<> inline core::Type core::GetTypeInfo<video::Color>() { return core::Type::Color; }

namespace math
{
template <>
inline video::Color Lerp(const video::Color& a, const video::Color& b, float t)
{
	assert(t >= 0.0f && t <= 1.0f);

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

}

}    // lux

#endif