#include "ImageLoaderBMP.h"
#include "math/Dimension2.h"
#include "video/Color.h"
#include "io/File.h"
#include "video/images/Image.h"

namespace lux
{
namespace video
{
namespace
{
#pragma pack(push, 1)
struct BITMAPFILEHEADER
{
	u16 type;
	u32 Size;
	u32 Reserved;
	u32 offset;
};

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
};

struct SColorMasks
{
	u32 mRed;
	u32 mGreen;
	u32 mBlue;
	u32 oRed;
	u32 oGreen;
	u32 oBlue;
};
#pragma pack(pop)

struct Context
{
	math::Dimension2I size;
	ColorFormat format;
	u8* colorTable;
	s64 dataOffset;
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;
	SColorMasks colorMasks;

	Context() :
		colorTable(nullptr),
		dataOffset(0)
	{
	}
};
}
static u32 GetRightZeros(u32 x)
{
	u32 o = 0;
	while(((x >> o) & 1) == 0)
		++o;

	return o;
}

static bool LoadImageFormat(Context& ctx, io::File* file)
{
	ctx.colorTable = nullptr;

	ctx.dataOffset = file->GetCursor();

	file->ReadBinary(sizeof(BITMAPFILEHEADER), &ctx.header);

	if(ctx.header.type != 0x4D42)
		return false;

	file->ReadBinary(sizeof(BITMAPINFOHEADER), &ctx.info);

	if(ctx.info.Size != 40 && ctx.info.Size != ctx.header.offset - sizeof(BITMAPFILEHEADER))
		return false;

	ctx.size.Set(ctx.info.Width, math::Abs(ctx.info.height));

	/*
	if(ctx.info.BitCount < 16)
		return false;
	*/

	if(ctx.info.Compression != 0 && ctx.info.Compression != 3)
		return false;

	if(ctx.info.Compression == 3) {
		if(ctx.info.BitCount != 32 && ctx.info.BitCount != 16)
			return false;

		file->ReadBinary(12, &ctx.colorMasks);
		if(ctx.info.BitCount == 32) {
			if(ctx.colorMasks.mRed == 0x00FF0000 && ctx.colorMasks.mGreen == 0x0000FF00 && ctx.colorMasks.mBlue == 0x000000FF)
				ctx.info.Compression = 0;
		} else {
			if(ctx.colorMasks.mRed == 0x7C00 && ctx.colorMasks.mGreen == 0x03E0 && ctx.colorMasks.mBlue == 0x001F)
				ctx.info.Compression = 0;
		}

		if(ctx.info.Compression == 3) {
			ctx.colorMasks.oRed = GetRightZeros(ctx.colorMasks.mRed);
			ctx.colorMasks.oGreen = GetRightZeros(ctx.colorMasks.mGreen);
			ctx.colorMasks.oBlue = GetRightZeros(ctx.colorMasks.mBlue);
		}
	}

	// Read Colortable
	if(ctx.info.BitCount <= 8) {
		if(ctx.info.ClrUsed == 0)
			ctx.info.ClrUsed = 1 << ctx.info.BitCount;

		ctx.colorTable = LUX_NEW_ARRAY(u8, ctx.info.ClrUsed * 4);
		file->ReadBinary(ctx.info.ClrUsed * 4, ctx.colorTable);
	}

	ctx.format = ColorFormat::R8G8B8;

	ctx.dataOffset += ctx.header.offset;

	return true;
}

static bool LoadImageToMemory(Context& ctx, io::File* file, void* dest)
{
	// Move to color data
	file->Seek(ctx.dataOffset, io::ESeekOrigin::Start);

	const int ImagePitch = ctx.size.width * ctx.format.GetBytePerPixel();
	const int ImageSize = ctx.size.height * ImagePitch;

	u8* cursor;
	int  pitch;
	if(ctx.info.height > 0) {
		cursor = (u8*)dest + ImageSize - ImagePitch;
		pitch = -ImagePitch;
	} else {
		cursor = (u8*)dest;
		pitch = ImagePitch;
	}

	int filePitch = (ctx.size.width * ctx.info.BitCount) / 8;

	if(filePitch % 4 != 0)
		filePitch = filePitch + (4 - filePitch % 4);    // Auf 4-Byte ausrichten

	u8* lineData = LUX_NEW_ARRAY(u8, filePitch);

	for(int j = 0; j < ctx.size.height; ++j) {
		// Read one line from the file
		file->ReadBinary(filePitch, lineData);

		u8* pD = (u8*)cursor;

		// Decode and write line to output
		if(ctx.info.BitCount == 32) {
			u32* pS = (u32*)lineData;

			if(ctx.info.Compression == 3) {
				for(int i = 0; i < ctx.size.width; ++i) {
					*(pD++) = (u8)((*pS & ctx.colorMasks.mRed) >> ctx.colorMasks.oRed);
					*(pD++) = (u8)((*pS & ctx.colorMasks.mGreen) >> ctx.colorMasks.oGreen);
					*(pD++) = (u8)((*pS & ctx.colorMasks.mBlue) >> ctx.colorMasks.oBlue);

					pS++;
				}
			} else {
				for(int i = 0; i < ctx.size.width; ++i) {
					*(pD++) = (u8)((*pS & 0x00FF0000) >> 16);
					*(pD++) = (u8)((*pS & 0x0000FF00) >> 8);
					*(pD++) = (u8)((*pS & 0x000000FF) >> 0);

					pS++;
				}
			}

			cursor += pitch;
		} else if(ctx.info.BitCount == 24) {
			u8* pS = (u8*)lineData;

			for(int i = 0; i < ctx.size.width; ++i) {
				// data is in BGR order
				*(pD++) = *(pS + 2);
				*(pD++) = *(pS + 1);
				*(pD++) = *(pS + 0);

				pS += 3;
			}

			cursor += pitch;
		} else if(ctx.info.BitCount == 16) {
			u16* pS = (u16*)lineData;

			if(ctx.info.Compression == 3) {
				for(int i = 0; i < ctx.size.width; ++i) {
					*(pD++) = (u8)((*pS & ctx.colorMasks.mRed) >> ctx.colorMasks.oRed);
					*(pD++) = (u8)((*pS & ctx.colorMasks.mGreen) >> ctx.colorMasks.oGreen);
					*(pD++) = (u8)((*pS & ctx.colorMasks.mBlue) >> ctx.colorMasks.oBlue);

					++pS;
				}
			} else {
				for(int i = 0; i < ctx.size.width; ++i) {
					*(pD++) = (u8)((*pS & 0x00007C00) >> 11);
					*(pD++) = (u8)((*pS & 0x000003E0) >> 5);
					*(pD++) = (u8)((*pS & 0x0000001F) >> 0);

					++pS;
				}
			}

			cursor += pitch;
		} else if(ctx.info.BitCount == 8) {
			u8* pS = (u8*)lineData;

			for(int i = 0; i < ctx.size.width; ++i) {
				u8 e = *pS;
				*(pD++) = ctx.colorTable[4 * e + 2];
				*(pD++) = ctx.colorTable[4 * e + 1];
				*(pD++) = ctx.colorTable[4 * e + 0];

				++pS;
			}

			cursor += pitch;
		} else if(ctx.info.BitCount == 4) {
			u8* pS = (u8*)lineData;

			for(int i = 0; i < ctx.size.width / 2; ++i) {
				u8 e = 4 * ((*pS >> 4) & 0x0F);
				*(pD++) = ctx.colorTable[e + 2];
				*(pD++) = ctx.colorTable[e + 1];
				*(pD++) = ctx.colorTable[e + 0];

				e = 4 * ((e >> 0) & 0x0F);
				*(pD++) = ctx.colorTable[e + 2];
				*(pD++) = ctx.colorTable[e + 1];
				*(pD++) = ctx.colorTable[e + 0];


				++pS;
			}

			if((ctx.size.width & 1) != 0) {
				u8 e = 4 * ((*pS & 0xF0) >> 4);

				*(pD++) = ctx.colorTable[e + 2];
				*(pD++) = ctx.colorTable[e + 1];
				*(pD++) = ctx.colorTable[e + 0];
			}

			cursor += pitch;
		} else if(ctx.info.BitCount == 1) {
			u8* pS = (u8*)lineData;
			for(int i = 0; i < ctx.size.width / 8; ++i) {
				u8 x = *pS;

				u8 e = ((x & 0x80) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x40) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x20) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x10) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x08) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x04) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x02) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
				e = ((x & 0x01) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];

				++pS;
			}

			u8 x = *pS;
			u8 e;
			switch(ctx.size.width % 8) {
			case 7:
				e = ((x & 0x02) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 6:
				e = ((x & 0x04) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 5:
				e = ((x & 0x08) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 4:
				e = ((x & 0x10) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 3:
				e = ((x & 0x20) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 2:
				e = ((x & 0x40) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			case 1:
				e = ((x & 0x80) != 0);
				*(pD++) = ctx.colorTable[e + 2]; *(pD++) = ctx.colorTable[e + 1]; *(pD++) = ctx.colorTable[e + 0];
			}

			cursor += pitch;
		}
	}

	LUX_FREE_ARRAY(lineData);
	LUX_FREE_ARRAY(ctx.colorTable);

	return true;
}


core::Name ImageLoaderBMP::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	u16 data;
	if(file->ReadBinaryPart(sizeof(data), &data) != sizeof(data))
		return core::Name::INVALID;
	if(data != 0x4D42)
		return core::Name::INVALID;
	return core::ResourceType::Image;
}

const core::String& ImageLoaderBMP::GetName() const
{
	static const core::String name = "Lux BMP-Loader";

	return name;
}

void ImageLoaderBMP::LoadResource(io::File* file, core::Resource* dst)
{
	bool result;

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(!img)
		throw core::InvalidOperationException("Passed wrong resource type to loader");

	Context ctx;
	result = LoadImageFormat(ctx, file);
	if(!result)
		throw core::FileFormatException("Failed to load", "bmp");

	img->Init(ctx.size, ctx.format);

	{
		video::ImageLock lock(img);
		result = LoadImageToMemory(ctx, file, lock.data);
	}
}

}
}