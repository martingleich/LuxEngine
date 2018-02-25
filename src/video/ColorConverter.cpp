#include "video/ColorConverter.h"
#include "core/lxMemory.h"

namespace lux
{
namespace video
{

static void Convert_R8G8B8toA8R8G8B8(const void* src, void* dst, u32 width, u32 height, u32 srcPitch, u32 dstPitch)
{
	u8* sB = (u8*)src;
	u32* dB = (u32*)dst;

	u32 srcLineAlign = srcPitch - 3 * width;
	u32 dstLineAlign = dstPitch - 4 * width;
	for(u32 l = 0; l < height; ++l) {
		for(u32 i = 0; i < width; ++i) {
			*dB = 0xFF000000 | (sB[0] << 16) | (sB[1] << 8) | sB[2];
			sB += 3;
			++dB;
		}
		sB += srcLineAlign;
		dB = (u32*)(((u8*)dB) + dstLineAlign);
	}


	/*
	width overlapping
	*/
	/*
	const u8* sB = (const u8*)src + (count-1)*3;
	u32* dB = (u32*)dst +(count-1);

	for(u32 i = 0; i < count; ++i)
	{
		*dB = 0xFF000000 | (sB[0] << 16) | (sB[1] << 8) | sB[2];
		sB -= 3;
		--dB;
	}
	*/
}

static void Convert_R8G8B8toA1R5G5B5(const void* src, void* dst, u32 width, u32 height, u32 srcPitch, u32 dstPitch)
{
	// width overlapping
	u8* sB = (u8*)src;
	u16* dB = (u16*)dst;

	u32 srcLineAlign = srcPitch - 3 * width;
	u32 dstLineAlign = dstPitch - 2 * width;
	for(u32 l = 0; l < height; ++l) {
		for(u32 i = 0; i < width; ++i) {
			u16 r = sB[0] >> 3;
			u16 g = sB[1] >> 3;
			u16 b = sB[2] >> 3;
			*dB = 0x8000 | (r << 10) | (g << 5) | b;
			sB += 3;
			++dB;
		}
		sB += srcLineAlign;
		dB = (u16*)((u8*)dB + dstLineAlign);
	}
}

static void Convert_Trivial(const void* src, void* dst, u32 width, u32 height, u32 srcPitch, u32 dstPitch, ColorFormat srcFormat, ColorFormat dstFormat)
{
	u8* sB = (u8*)src;
	u8* dB = (u8*)dst;

	// Assert on overlap
	lxAssert(!(sB < (dB + height*dstPitch) && dB < (sB + height*srcPitch)));

	u32 sizeS = srcFormat.GetBytePerPixel();
	u32 sizeD = dstFormat.GetBytePerPixel();

	u32 srcLineAlign = srcPitch - sizeS*width;
	u32 dstLineAlign = dstPitch - sizeD*width;

	for(u32 l = 0; l < height; ++l) {
		for(u32 i = 0; i < width; ++i) {
			u32 t = srcFormat.FormatToA8R8G8B8(sB);
			dstFormat.A8R8G8B8ToFormat(t, dB);

			sB += sizeS;
			dB += sizeD;
		}
		sB += srcLineAlign;
		dB += dstLineAlign;
	}
}

static void CopyImage(const void* src, void* dst, ColorFormat format, u32 width, u32 height, u32 pitch)
{
	if(dst != src) {
		u8* dB = (u8*)dst;
		const u8* sB = (const u8*)src;
		for(u32 l = 0; l < height; ++l) {
			memcpy(dB, sB, (format.GetBitsPerPixel() * width)/8);
			dB += pitch;
			sB += pitch;
		}
	}
}

bool ColorConverter::IsConvertable(ColorFormat srcFormat, ColorFormat dstFormat)
{
	if(srcFormat == dstFormat)
		return true;
	if(srcFormat.IsFloatingPoint() || dstFormat.IsFloatingPoint() || srcFormat.IsCompressed() || dstFormat.IsCompressed())
		return false;

	return true;
}

bool ColorConverter::ConvertByFormat(
	const void* src, ColorFormat srcFormat,
	void* dst, ColorFormat dstFormat,
	u32 width, u32 height, u32 srcPitch, u32 dstPitch)
{
	if(!IsConvertable(srcFormat, dstFormat))
		return false;

	if(srcPitch < width * srcFormat.GetBytePerPixel())
		srcPitch = width * srcFormat.GetBytePerPixel();
	if(dstPitch == width * dstFormat.GetBytePerPixel())
		dstPitch = width * dstFormat.GetBytePerPixel();

	if(srcFormat == dstFormat)
		CopyImage(src, dst, srcFormat, width, height, srcPitch);
	else if(srcFormat == ColorFormat::R8G8B8 && dstFormat == ColorFormat::A8R8G8B8)
		Convert_R8G8B8toA8R8G8B8(src, dst, width, height, srcPitch, dstPitch);
	else if(srcFormat == ColorFormat::R8G8B8 && dstFormat == ColorFormat::A1R5G5B5)
		Convert_R8G8B8toA1R5G5B5(src, dst, width, height, srcPitch, dstPitch);
	else
		Convert_Trivial(src, dst, width, height, srcPitch, dstPitch, srcFormat, dstFormat);

	return true;
}

}
}