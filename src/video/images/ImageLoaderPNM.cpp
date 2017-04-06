#include "ImageLoaderPNM.h"
#include "io/File.h"

namespace lux
{
namespace video
{

char ImageLoaderPNM::GetChar()
{
	if(m_File->IsEOF()) {
		m_Error = true;
		return 0;
	} else {
		return m_File->Read<char>();
	}
}

u32 ImageLoaderPNM::GetNext(bool Header)
{
	if(m_Error)
		return 0;

	if(Header || m_Ascii) {
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

void ImageLoaderPNM::ReadHeader()
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

int ImageLoaderPNM::ReadMagic()
{
	char magic[2];
	if(m_File->ReadBinary(2, magic) != 2)
		return -1;
	if(magic[0] != 'P')
		return -1;
	if(magic[1] < '1' || magic[1] > '6')
		return -1;

	return magic[1] - '0';
}

void ImageLoaderPNM::LoadGreyMap(u8* Dest)
{
	int PixCount = m_Width * m_Height;
	for(int i = 0; i < PixCount; ++i) {
		u32 v = GetNext();
		if(m_Error)
			return;
		u8 c = (u8)((v * 255) / m_MaxValue);
		*(Dest++) = c;
		*(Dest++) = c;
		*(Dest++) = c;
	}
}

void ImageLoaderPNM::LoadBitMap(u8* Dest)
{
	if(m_Ascii)
		LoadGreyMap(Dest);

	for(int i = 0; i < (int)m_Height; ++i) {
		for(int j = 0; j < (int)((m_Width + 1) / 8); ++j) {
			u32 v = GetNext();
			if(m_Error)
				return;
			const int Bits = (j * 8 + 8 > (int)m_Width) ? m_Width % 8 : 8;
			u8 Mask = 0x80;
			u8 shift = 7;
			for(int k = 0; k < Bits; ++k) {
				u8 c = ((v&Mask) >> shift) * 255;
				*(Dest++) = c;
				*(Dest++) = c;
				*(Dest++) = c;
				Mask >>= 1;
				shift--;
			}
		}
	}
}

void ImageLoaderPNM::LoadPixMap(u8* Dest)
{
	int PixCount = m_Width * m_Height;
	for(int i = 0; i < PixCount; ++i) {
		*(Dest++) = (u8)((GetNext() * 255) / m_MaxValue);
		*(Dest++) = (u8)((GetNext() * 255) / m_MaxValue);
		*(Dest++) = (u8)((GetNext() * 255) / m_MaxValue);
		if(m_Error)
			return;
	}
}

core::Name ImageLoaderPNM::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	m_File = file;
	if(ReadMagic() < 0)
		return core::Name::INVALID;
	else
		return core::ResourceType::Image;
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

	result = LoadImageFormat(file);
	if(!result)
		return false;

	img->Init(
		math::dimension2du(m_Width,m_Height),
		m_OutputFormat);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(file, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}

bool ImageLoaderPNM::LoadImageFormat(io::File* file)
{
	if(!file)
		return false;

	m_File = file;
	m_Error = false;

	ReadHeader();

	if(m_Error)
		return false;

	m_OutputFormat = video::ColorFormat::R8G8B8;

	return true;
}

bool ImageLoaderPNM::LoadImageToMemory(io::File* file, void* dest)
{
	if(!file)
		return false;
	if(!dest)
		return false;

	switch(m_Type) {
	case BITMAP:
		m_NativeFormat = ColorFormat::X8;
		LoadBitMap((u8*)dest);
		break;
	case GREYMAP:
		m_NativeFormat = ColorFormat::X8;
		LoadGreyMap((u8*)dest);
		break;
	case PIXMAP:
		m_NativeFormat = ColorFormat::R8G8B8;
		LoadPixMap((u8*)dest);
		break;
	}

	return !m_Error;
}

}
}