#include "ImageWriterBMP.h"
#include "io/File.h"

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

bool ImageWriterBMP::CanWriteFile(const io::path& file)
{
	string ext = io::GetFileExtension(file);
	return ext.EqualCaseInsensitive("bmp") || ext.EqualCaseInsensitive("dib");
}

bool ImageWriterBMP::WriteFile(io::File* File, void* Data, video::ColorFormat Format, math::dimension2du Size, u32 Pitch, u32 WriterParam)
{
	LUX_UNUSED(WriterParam);

	if(!File)
		return false;
	if(!Data)
		return false;

	if(Size.GetArea() == 0)
		return false;

	int FileStartPosition = File->GetCursor();

	BITMAPFILEHEADER Header;
	Header.Type = 0x4D42;
	Header.Size = 14 + 40 + 3 * Size.GetArea();
	Header.Reserved = 0;
	Header.Offset = 54;
	if(File->WriteBinary(&Header, sizeof(Header)) != sizeof(Header)) {
		File->Seek(FileStartPosition, io::ESeekOrigin::Start);
		return false;
	}

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
	if(File->WriteBinary(&Info, sizeof(Info)) != sizeof(Info)) {
		File->Seek(FileStartPosition, io::ESeekOrigin::Start);
		return false;
	}

	const int BytePerPixel = Format.GetBytePerPixel();
	u8* DataCursor = (u8*)Data + (Size.height - 1) * Pitch;
	u32 Len = 3 * Size.width;
	if(Len % 4 != 0)
		Len += 4 - (Len % 4);
	u8* line = LUX_NEW_ARRAY(u8, Len);
	*((u32*)(line + Len - 4)) = 0;
	u8* LineCursor = line;

	for(u32 i = 0; i < Size.height; ++i) {
		for(u32 j = 0; j < Size.width; ++j) {
			u32 pixel = Format.FormatToA8R8G8B8(DataCursor);
			LineCursor[0] = (u8)((pixel & 0x000000FF) >> 0);
			LineCursor[1] = (u8)((pixel & 0x0000FF00) >> 8);
			LineCursor[2] = (u8)((pixel & 0x00FF0000) >> 16);

			LineCursor += 3;
			DataCursor += BytePerPixel;
		}

		if(File->WriteBinary(line, Len) != Len) {
			LUX_FREE_ARRAY(line);
			File->Seek(FileStartPosition, io::ESeekOrigin::Start);
			return false;
		}

		DataCursor -= Size.width*Format.GetBytePerPixel();
		DataCursor -= Pitch;
		LineCursor = line;
	}

	LUX_FREE_ARRAY(line);

	return true;
}


}
}