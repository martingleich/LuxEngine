#ifndef INCLUDED_IMAGELOADERPNM_H
#define INCLUDED_IMAGELOADERPNM_H
#include "resources/ResourceLoader.h"
#include "video/images/Image.h"

namespace lux
{
namespace video
{

class ImageLoaderPNM : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const string& GetName() const;

	bool LoadResource(io::File* file, core::Resource* dst);
	bool LoadImageFormat(io::File* file);
	bool LoadImageToMemory(io::File* file, void* dest);

private:
	char GetChar();
	u32 GetNext(bool Header = false);
	void ReadHeader();
	int ReadMagic();
	void LoadGreyMap(u8* Dest);
	void LoadBitMap(u8* Dest);
	void LoadPixMap(u8* Dest);

private:
	enum EType
	{
		PIXMAP,
		GREYMAP,
		BITMAP,
	};

private:
	bool m_Ascii;
	u32 m_Width;
	u32 m_Height;
	int m_MaxValue;
	EType m_Type;

	bool m_Error;

	io::File* m_File;

	ColorFormat m_NativeFormat;
	ColorFormat m_OutputFormat;
};

}
}

#endif