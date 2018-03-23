#ifndef INCLUDED_LUX_COLORCONVERTER_H
#define INCLUDED_LUX_COLORCONVERTER_H
#include "video/Color.h"

namespace lux
{
namespace video
{

//! Convert image data between formats
/**
This function can convert a lot faster than single calls to other conversion functions
*/
class ColorConverter
{
public:
	//! Are two formats convertible
	/**
	\param srcFormat The source format.
	\param dstFormat The desination format.
	\return Is the source format in the destination format convertable.
	*/
	LUX_API static bool IsConvertable(ColorFormat srcFormat, ColorFormat dstFormat);

	//! Convert image data from one format into another
	/**
	\param src A pointer to the source data.
	\param srcFormat The format of the source data.
	\param dst A pointer to the desination.
	\param dstFormat The format of the new data.
	\param width The width of the image.
	\param height The height of image.
	\param srcPitch The pitch of the source image.
	\param dstPitch The pitch of the destination image.
	*/
	LUX_API static bool ConvertByFormat(
		const void* src, ColorFormat srcFormat,
		void* dst, ColorFormat dstFormat,
		u32 width, u32 height, u32 srcPitch = 0, u32 dstPitch = 0);
};

} // !namespace video
} // !namespace lux

#endif // !INCLUDED_LUX_CCOLORCONVERTER_H
