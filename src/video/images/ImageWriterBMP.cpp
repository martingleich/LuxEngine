#include "ImageWriterBMP.h"
#include "io/File.h"
#include "core/lxMemory.h"

namespace lux
{
namespace video
{

#pragma pack(push, 1)
struct BITMAPFILEHEADER
{
	u16 Type;
	u32 Size;
	u32 Reserved;
	u32 Offset;
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
#pragma pack(pop)

bool ImageWriterBMP::CanWriteFile(const core::String& ext)
{
	return ext.IsEmpty() || ext.Equal("bmp", core::EStringCompare::CaseInsensitive) || ext.Equal("dib", core::EStringCompare::CaseInsensitive);
}

void ImageWriterBMP::WriteFile(io::File* File, void* Data, video::ColorFormat Format, math::Dimension2I Size, u32 Pitch, u32 WriterParam)
{
	LUX_UNUSED(WriterParam);

	LX_CHECK_NULL_ARG(File);
	LX_CHECK_NULL_ARG(Data);

	if(Size.GetArea() == 0)
		throw core::InvalidArgumentException("Size", "The size of the image must be greater than 0");

	BITMAPFILEHEADER Header;
	Header.Type = 0x4D42;
	Header.Size = 14 + 40 + 3 * Size.GetArea();
	Header.Reserved = 0;
	Header.Offset = 54;
	File->WriteBinary(&Header, sizeof(Header));

	BITMAPINFOHEADER Info;
	Info.Size = 40;
	Info.Width = Size.width;
	Info.height = Size.height;
	Info.Planes = 1;
	Info.BitCount = 24;	// We always write R8G8B8
	Info.Compression = 0;
	Info.SizeImage = 3 * Size.GetArea();
	Info.XPelsPerMeter = 0;
	Info.YPelsPerMeter = 0;
	Info.ClrUsed = 0;
	Info.ClrImportant = 0;
	File->WriteBinary(&Info, sizeof(Info));

	const int BytePerPixel = Format.GetBytePerPixel();
	u8* DataCursor = (u8*)Data + (Size.height - 1) * Pitch;
	int Len = 3 * Size.width;
	if(Len % 4 != 0)
		Len += 4 - (Len % 4);
	core::RawMemory lineMem(Len);
	u8* line = lineMem;
	*((u32*)(line + Len - 4)) = 0;
	u8* LineCursor = line;

	for(int i = 0; i < Size.height; ++i) {
		for(int j = 0; j < Size.width; ++j) {
			u32 pixel = Format.FormatToA8R8G8B8(DataCursor);
			LineCursor[0] = (u8)((pixel & 0x000000FF) >> 0);
			LineCursor[1] = (u8)((pixel & 0x0000FF00) >> 8);
			LineCursor[2] = (u8)((pixel & 0x00FF0000) >> 16);

			LineCursor += 3;
			DataCursor += BytePerPixel;
		}

		File->WriteBinary(line, Len);

		DataCursor -= Size.width*Format.GetBytePerPixel();
		DataCursor -= Pitch;
		LineCursor = line;
	}
}


}
}