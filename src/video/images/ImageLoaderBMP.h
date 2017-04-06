#ifndef INCLUDED_IMAGELOADERBMP_H
#define INCLUDED_IMAGELOADERBMP_H
#include "resources/ResourceLoader.h"
#include "math/dimension2d.h"
#include "video/Color.h"

//#define USE_ALPHA_CHANNEL

namespace lux
{
namespace video
{

class ImageLoaderBMP : public core::ResourceLoader
{
public:
	ImageLoaderBMP();
	~ImageLoaderBMP();

	bool LoadResource(io::File* file, core::Resource* dst);
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const string& GetName() const;

	bool LoadImageFormat(io::File* file);
	bool LoadImageToMemory(io::File* file, void* dest);

private:
	u32 GetRightZeros(u32 x);

private:
#pragma pack(push, 1)
	struct BITMAPFILEHEADER
	{
		u16 type;
		u32 Size;
		u32 Reserved;
		u32 offset;
	} m_Header;

	struct BITMAPINFOHEADER
	{
		u32 Size;
		s32 Width;
		s32 height;
		u16 Planes;
		u16 BitCount;
		u32 Compression;
		u32 SizeImage;
		s32 XPelsPerMeter;
		s32 YPelsPerMeter;
		u32 ClrUsed;
		u32 ClrImportant;
	} m_Info;

	struct SColorMasks
	{
		u32 mRed;
		u32 mGreen;
		u32 mBlue;
		u32 oRed;
		u32 oGreen;
		u32 oBlue;
	} m_ColorMasks;
#pragma pack(pop)

private:
	math::dimension2du m_Size;
	ColorFormat m_Format;
	u8* m_ColorTable;
	u32 m_DataOffset;

};

}
}

#endif // !INCLUDED_CIMAGELOADERBMP_H
