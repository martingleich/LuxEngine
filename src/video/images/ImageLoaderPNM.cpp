#include "ImageLoaderPNM.h"
#include "io/File.h"
#include "video/images/Image.h"

namespace lux
{
namespace video
{

namespace
{
enum EType
{
	PIXMAP,
	GREYMAP,
	BITMAP,
};

struct Context
{
	bool m_Ascii;
	u32 m_Width;
	u32 m_Height;
	int m_MaxValue;
	EType m_Type;

	bool m_Error;

	io::File* m_File;

	ColorFormat m_NativeFormat;
	ColorFormat m_OutputFormat;

	char GetChar()
	{
		if(m_File->IsEOF()) {
			m_Error = true;
			return 0;
		} else {
			return m_File->Read<char>();
		}
	}

	u32 GetNext(bool header=false)
	{
		if(m_Error)
			return 0;

		if(header || m_Ascii) {
			char c;
			do {
				c = GetChar();
				if(c == '#') {
					while(GetChar() != '\n') {
						if(m_Error)
							return 0;
					}
				} else if(core::IsDigit(c)) {
					u32 Number = c - '0';
					while(core::IsDigit(c = GetChar())) {
						Number *= 10;
						Number += c - '0';
					}
					if(m_Error)
						return 0;

					return Number;
				}
			} while(c != 0);
		} else {
			return GetChar();
		}

		m_Error = true;
		return 0;
	}

	void ReadHeader()
	{
		int type = ReadMagic();
		m_Ascii = (type <= 3);
		if(type == 1 || type == 4)
			m_Type = BITMAP;
		if(type == 2 || type == 5)
			m_Type = GREYMAP;
		if(type == 3 || type == 6)
			m_Type = PIXMAP;

		m_Width = GetNext(true);
		m_Height = GetNext(true);
		if(m_Type != BITMAP) {
			m_MaxValue = GetNext(true);
			if(m_MaxValue > 255)
				m_Error = true;
		} else {
			m_MaxValue = 1;
		}
	}

	static int ReadMagic(io::File* f)
	{
		char magic[2];
		if(f->ReadBinary(2, magic) != 2)
			return -1;
		if(magic[0] != 'P')
			return -1;
		if(magic[1] < '1' || magic[1] > '6')
			return -1;

		return magic[1] - '0';
	}

	int ReadMagic()
	{
		return ReadMagic(m_File);
	}

	void LoadGreyMap(u8* dest)
	{
		u32 pixCount = m_Width * m_Height;
		for(u32 i = 0; i < pixCount; ++i) {
			u32 v = GetNext();
			if(m_Error)
				return;
			u8 c = (u8)((v * 255) / m_MaxValue);
			*(dest++) = c;
			*(dest++) = c;
			*(dest++) = c;
		}
	}

	void LoadBitMap(u8* dest)
	{
		if(m_Ascii)
			LoadGreyMap(dest);

		for(u32 i = 0; i < m_Height; ++i) {
			for(u32 j = 0; j < ((m_Width + 1) / 8); ++j) {
				u32 v = GetNext();
				if(m_Error)
					return;
				const u32 Bits = (j * 8 + 8 > m_Width) ? m_Width % 8 : 8;
				u8 mask = 0x80;
				u8 shift = 7;
				for(u32 k = 0; k < Bits; ++k) {
					u8 c = ((v&mask) >> shift) * 255;
					*(dest++) = c;
					*(dest++) = c;
					*(dest++) = c;
					mask >>= 1;
					shift--;
				}
			}
		}
	}

	void LoadPixMap(u8* dest)
	{
		u32 pixCount = m_Width * m_Height;
		for(u32 i = 0; i < pixCount; ++i) {
			*(dest++) = (u8)((GetNext() * 255) / m_MaxValue);
			*(dest++) = (u8)((GetNext() * 255) / m_MaxValue);
			*(dest++) = (u8)((GetNext() * 255) / m_MaxValue);
			if(m_Error)
				return;
		}
	}
};

}

core::Name ImageLoaderPNM::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	if(Context::ReadMagic(file) < 0)
		return core::Name::INVALID;
	else
		return core::ResourceType::Image;
}

static bool LoadImageFormat(Context& ctx, io::File* file)
{
	if(!file)
		return false;

	ctx.m_File = file;
	ctx.m_Error = false;

	ctx.ReadHeader();

	if(ctx.m_Error)
		return false;

	ctx.m_OutputFormat = video::ColorFormat::R8G8B8;

	return true;
}

static bool LoadImageToMemory(Context& ctx, void* dest)
{
	switch(ctx.m_Type) {
	case BITMAP:
		ctx.m_NativeFormat = ColorFormat::X8;
		ctx.LoadBitMap((u8*)dest);
		break;
	case GREYMAP:
		ctx.m_NativeFormat = ColorFormat::X8;
		ctx.LoadGreyMap((u8*)dest);
		break;
	case PIXMAP:
		ctx.m_NativeFormat = ColorFormat::R8G8B8;
		ctx.LoadPixMap((u8*)dest);
		break;
	}

	return !ctx.m_Error;
}

const string& ImageLoaderPNM::GetName() const
{
	static const string name = "Lux PNM Loader";

	return name;
}

bool ImageLoaderPNM::LoadResource(io::File* file, core::Resource* dst)
{
	bool result;

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(!img)
		return false;

	Context ctx;
	result = LoadImageFormat(ctx, file);
	if(!result)
		return false;

	img->Init(
		math::dimension2du(ctx.m_Width, ctx.m_Height),
		ctx.m_OutputFormat);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(ctx, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}


}
}