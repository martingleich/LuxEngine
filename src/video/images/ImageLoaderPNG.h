#pragma once
#ifndef INCLUDED_IMAGE_LOADER_PNG_H
#define INCLUDED_IMAGE_LOADER_PNG_H
#include "resources/ResourceLoader.h"
#include "math/dimension2d.h"
#include "video/Color.h"

typedef struct png_info_def png_info;
typedef struct png_struct_def png_struct;

namespace lux
{
namespace video
{

class ImageLoaderPNG : public core::ResourceLoader
{
public:
	struct MainProgInfo
	{
		jmp_buf jmpbuf;
	};

public:
	ImageLoaderPNG() :
		m_Png(nullptr),
		m_PngInfo(nullptr)
	{}

	const string& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	bool LoadResource(io::File* file, core::Resource* dst);

	u32 LoadImageCount(io::File* file);
	bool LoadImageFormat(io::File* file);
	bool LoadImageToMemory(io::File* file, void* dst);
	void Init(io::File* file);
	void Exit();

private:
	io::File* m_CurFile;

	png_struct* m_Png;
	png_info* m_PngInfo;
	MainProgInfo m_Info;

	int m_Depth;
	int m_ColorType;
	u32 m_Width;
	u32 m_Height;
	ColorFormat m_Format;
};

}
}

#endif // !INCLUDED_IMAGE_LOADER_PNG_H