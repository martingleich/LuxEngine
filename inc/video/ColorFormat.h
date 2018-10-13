#ifndef INCLUDED_LUX_COLOR_FORMAT_H
#define INCLUDED_LUX_COLOR_FORMAT_H
#include "core/lxException.h"
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
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) name,
#include "ColorFormat.def"
#undef X
	};

	//! The number of available formats
	static const u32 FORMAT_COUNT = 19;

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
	EColorFormat ToEnum() const
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
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) #name,
#include "ColorFormat.def"
#undef X
		};
		return STRINGS[m_Format];
	}

	//! The number of bits in a pixel for this format
	inline int GetBitsPerPixel() const
	{
		static int BitPerPixel[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (size),
#include "ColorFormat.def"
#undef X
		};
			return BitPerPixel[(u32)m_Format];
	}

	//! The number of bytes in a pixel for this format
	inline int GetBytePerPixel() const
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
		static u32 Mask[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (red_mask),
#include "ColorFormat.def"
#undef X
		};
		return Mask[(u32)m_Format];
	}

	//! The mask for the green channel
	/**
	Binary anding a pixel with this value keeps only the green pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetGreenMask() const
	{
		static u32 Mask[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (green_mask),
#include "ColorFormat.def"
#undef X
		};
		return Mask[(u32)m_Format];
	}

	//! The mask for the blue channel
	/**
	Binary anding a pixel with this value keeps only the blue pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetBlueMask() const
	{
		static u32 Mask[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (blue_mask),
#include "ColorFormat.def"
#undef X
		};
		return Mask[(u32)m_Format];
	}

	//! The mask for the alpha channel
	/**
	Binary anding a pixel with this value keeps only the alpha pixels. <br>
	Remark: This function only works for non floating point formats
	*/
	inline u32 GetAlphaMask() const
	{
		static u32 Mask[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (alpha_mask),
#include "ColorFormat.def"
#undef X
		};
		return Mask[(u32)m_Format];
	}

	//! Can this format contain a alpha value
	inline bool HasAlpha() const
	{
		if(m_Format == A16B16G16R16F ||
			m_Format == A32B32G32R32F)
			return true;
		else
			return GetAlphaMask() != 0;
	}

	inline int GetType() const
	{
		static u32 Type[] = {
#define X(name, alpha_mask, red_mask, green_mask, blue_mask, type, size) (type),
#include "ColorFormat.def"
#undef X
		};
		return Type[(u32)m_Format];
	}

	//! Is this format a floating point value
	inline bool IsFloatingPoint() const
	{
		return GetType() == 2;
	}
	//! Is this format compressed
	inline bool IsCompressed() const
	{
		return GetType() == 3;
	}

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
		case X8R8G8B8:
		case A8R8G8B8:
			out = *((u32*)(in));
			break;
		case X1R5G5B5:
		case A1R5G5B5:
			in16 = *((u16*)in);
			out = ((((u32)in16 & 0x8000) >> 31) & 0xFF00) |
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
			lxAssertNeverReach("Colorformat not implemented.");
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
		case X8R8G8B8:
		case A8R8G8B8:
			*(u32*)out = in;
			break;
		case X8:
			*out = (((in >> 16) & 0xFF) * 76 + ((in >> 8) & 0xFF) * 150 + (in & 0xFF) * 29) / 255;
			break;
		case X16:
			*(u16*)out = static_cast<u16>((((in >> 16) & 0xFF) * 19595 + ((in >> 8) & 0xFF) * 38469 + (in & 0xFF) * 7471) / 255);
			break;
		case X1R5G5B5:
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
			lxAssertNeverReach("Colorformat not implemented.");
			memset(out, 0, GetBytePerPixel());
		}
	}

private:
	EColorFormat m_Format;
};

inline void fmtPrint(format::Context& ctx, ColorFormat format, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	ctx.AddTerminatedSlice(format.AsString());
}

}

namespace core
{
struct UnsupportedColorFormatException : RuntimeException
{
	explicit UnsupportedColorFormatException(video::ColorFormat _format) :
		format(_format)
	{
	}

	video::ColorFormat GetColorFormat() const { return format; }
	ExceptionSafeString What() const { return ExceptionSafeString("UnsupportedColorFormatException: ").Append(format.AsString()); }
private:
	video::ColorFormat format;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_COLOR_FORMAT_H