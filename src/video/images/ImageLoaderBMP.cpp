#include "ImageLoaderBMP.h"
#include "io/File.h"
#include "video/images/Image.h"

namespace lux
{
namespace video
{

ImageLoaderBMP::ImageLoaderBMP() :
	m_ColorTable(nullptr),
	m_DataOffset(0)
{
}

ImageLoaderBMP::~ImageLoaderBMP()
{
}

u32 ImageLoaderBMP::GetRightZeros(u32 x)
{
	u32 o = 0;
	while(((x >> o) & 1) == 0)
		++o;

	return o;
}

core::Name ImageLoaderBMP::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	u16 data;
	if(file->ReadBinary(sizeof(data), &data) != sizeof(data))
		return core::Name::INVALID;
	if(data != 0x4D42)
		return core::Name::INVALID;
	return core::ResourceType::Image;
}

const string& ImageLoaderBMP::GetName() const
{
	static const string name = "Lux BMP-Loader";

	return name;
}


bool ImageLoaderBMP::LoadResource(io::File* file, core::Resource* dst)
{
	bool result;

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(!img)
		return false;

	result = LoadImageFormat(file);
	if(!result)
		return false;

	img->Init(m_Size, m_Format);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(file, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}

bool ImageLoaderBMP::LoadImageFormat(io::File* file)
{
	m_ColorTable = nullptr;

	m_DataOffset = file->GetCursor();

	file->ReadBinary(sizeof(BITMAPFILEHEADER), &m_Header);

	if(m_Header.type != 0x4D42)
		return false;

	file->ReadBinary(sizeof(BITMAPINFOHEADER), &m_Info);

	if(m_Info.Size != 40 && m_Info.Size != m_Header.offset - sizeof(BITMAPFILEHEADER))
		return false;

	m_Size.Set(m_Info.Width, math::Abs(m_Info.height));

	/*
	if(m_Info.BitCount < 16)
		return false;
	*/

	if(m_Info.Compression != 0 && m_Info.Compression != 3)
		return false;

	if(m_Info.Compression == 3) {
		if(m_Info.BitCount != 32 && m_Info.BitCount != 16)
			return false;

		file->ReadBinary(12, &m_ColorMasks);
		if(m_Info.BitCount == 32) {
			if(m_ColorMasks.mRed == 0x00FF0000 && m_ColorMasks.mGreen == 0x0000FF00 && m_ColorMasks.mBlue == 0x000000FF)
				m_Info.Compression = 0;
		} else {
			if(m_ColorMasks.mRed == 0x7C00 && m_ColorMasks.mGreen == 0x03E0 && m_ColorMasks.mBlue == 0x001F)
				m_Info.Compression = 0;
		}

		if(m_Info.Compression == 3) {
			m_ColorMasks.oRed = GetRightZeros(m_ColorMasks.mRed);
			m_ColorMasks.oGreen = GetRightZeros(m_ColorMasks.mGreen);
			m_ColorMasks.oBlue = GetRightZeros(m_ColorMasks.mBlue);
		}
	}

	// Read Colortable
	if(m_Info.BitCount <= 8) {
		if(m_Info.ClrUsed == 0)
			m_Info.ClrUsed = 1 << m_Info.BitCount;

		m_ColorTable = LUX_NEW_ARRAY(u8, m_Info.ClrUsed * 4);
		file->ReadBinary(m_Info.ClrUsed * 4, m_ColorTable);
	}

	m_Format = ColorFormat::R8G8B8;

	m_DataOffset += m_Header.offset;

	return true;
}

bool ImageLoaderBMP::LoadImageToMemory(io::File* file, void* dest)
{
	if(!file)
		return false;
	if(!dest)
		return false;

	// Move to color data
	file->Seek(m_DataOffset, io::ESeekOrigin::Start);

	const int ImagePitch = m_Size.width * m_Format.GetBytePerPixel();
	const int ImageSize = m_Size.height * ImagePitch;

	u8* cursor;
	int  pitch;
	if(m_Info.height > 0) {
		cursor = (u8*)dest + ImageSize - ImagePitch;
		pitch = -ImagePitch;
	} else {
		cursor = (u8*)dest;
		pitch = ImagePitch;
	}

	u32 filePitch = (m_Size.width * m_Info.BitCount) / 8;

	if(filePitch % 4 != 0)
		filePitch = filePitch + (4 - filePitch % 4);    // Auf 4-Byte ausrichten

	u8* lineData = LUX_NEW_ARRAY(u8, filePitch);

	for(u32 j = 0; j < m_Size.height; ++j) {
		// Read one line from the file
		file->ReadBinary(filePitch, lineData);

		u8* pD = (u8*)cursor;

		// Decode and write line to output
		if(m_Info.BitCount == 32) {
			u32* pS = (u32*)lineData;

			if(m_Info.Compression == 3) {
				for(u32 i = 0; i < m_Size.width; ++i) {
					*(pD++) = (u8)((*pS & m_ColorMasks.mRed) >> m_ColorMasks.oRed);
					*(pD++) = (u8)((*pS & m_ColorMasks.mGreen) >> m_ColorMasks.oGreen);
					*(pD++) = (u8)((*pS & m_ColorMasks.mBlue) >> m_ColorMasks.oBlue);

					pS++;
				}
			} else {
				for(u32 i = 0; i < m_Size.width; ++i) {
					*(pD++) = (u8)((*pS & 0x00FF0000) >> 16);
					*(pD++) = (u8)((*pS & 0x0000FF00) >> 8);
					*(pD++) = (u8)((*pS & 0x000000FF) >> 0);

					pS++;
				}
			}

			cursor += pitch;
		} else if(m_Info.BitCount == 24) {
			u8* pS = (u8*)lineData;

			for(u32 i = 0; i < m_Size.width; ++i) {
				// data is in BGR order
				*(pD++) = *(pS + 2);
				*(pD++) = *(pS + 1);
				*(pD++) = *(pS + 0);

				pS += 3;
			}

			cursor += pitch;
		} else if(m_Info.BitCount == 16) {
			u16* pS = (u16*)lineData;

			if(m_Info.Compression == 3) {
				for(u32 i = 0; i < m_Size.width; ++i) {
					*(pD++) = (u8)((*pS & m_ColorMasks.mRed) >> m_ColorMasks.oRed);
					*(pD++) = (u8)((*pS & m_ColorMasks.mGreen) >> m_ColorMasks.oGreen);
					*(pD++) = (u8)((*pS & m_ColorMasks.mBlue) >> m_ColorMasks.oBlue);

					++pS;
				}
			} else {
				for(u32 i = 0; i < m_Size.width; ++i) {
					*(pD++) = (u8)((*pS & 0x00007C00) >> 11);
					*(pD++) = (u8)((*pS & 0x000003E0) >> 5);
					*(pD++) = (u8)((*pS & 0x0000001F) >> 0);

					++pS;
				}
			}

			cursor += pitch;
		} else if(m_Info.BitCount == 8) {
			u8* pS = (u8*)lineData;

			for(u32 i = 0; i < m_Size.width; ++i) {
				u8 e = *pS;
				*(pD++) = m_ColorTable[4 * e + 2];
				*(pD++) = m_ColorTable[4 * e + 1];
				*(pD++) = m_ColorTable[4 * e + 0];

				++pS;
			}

			cursor += pitch;
		} else if(m_Info.BitCount == 4) {
			u8* pS = (u8*)lineData;

			for(u32 i = 0; i < m_Size.width / 2; ++i) {
				u8 e = 4 * ((*pS >> 4) & 0x0F);
				*(pD++) = m_ColorTable[e + 2];
				*(pD++) = m_ColorTable[e + 1];
				*(pD++) = m_ColorTable[e + 0];

				e = 4 * ((e >> 0) & 0x0F);
				*(pD++) = m_ColorTable[e + 2];
				*(pD++) = m_ColorTable[e + 1];
				*(pD++) = m_ColorTable[e + 0];


				++pS;
			}

			if((m_Size.width & 1) != 0) {
				u8 e = 4 * ((*pS & 0xF0) >> 4);

				*(pD++) = m_ColorTable[e + 2];
				*(pD++) = m_ColorTable[e + 1];
				*(pD++) = m_ColorTable[e + 0];
			}

			cursor += pitch;
		} else if(m_Info.BitCount == 1) {
			u8* pS = (u8*)lineData;
			for(u32 i = 0; i < m_Size.width / 8; ++i) {
				u8 x = *pS;

				u8 e = ((x & 0x80) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x40) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x20) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x10) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x08) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x04) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x02) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
				e = ((x & 0x01) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];

				++pS;
			}

			u8 x = *pS;
			u8 e;
			switch(m_Size.width % 8) {
			case 7:
				e = ((x & 0x02) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 6:
				e = ((x & 0x04) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 5:
				e = ((x & 0x08) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 4:
				e = ((x & 0x10) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 3:
				e = ((x & 0x20) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 2:
				e = ((x & 0x40) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			case 1:
				e = ((x & 0x80) != 0);
				*(pD++) = m_ColorTable[e + 2]; *(pD++) = m_ColorTable[e + 1]; *(pD++) = m_ColorTable[e + 0];
			}

			cursor += pitch;
		}
	}

	LUX_FREE_ARRAY(lineData);
	LUX_FREE_ARRAY(m_ColorTable);

	return true;
}
}
}