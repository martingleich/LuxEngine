#ifndef INCLUDED_IMAGELOADER_TGA
#define INCLUDED_IMAGELOADER_TGA
#include "resources/ResourceLoader.h"
#include "math/dimension2d.h"
#include "video/Color.h"

typedef struct tga_struct_t tga_struct;
typedef struct tga_info_t tga_info;

namespace lux
{
namespace video
{

class ImageLoaderTGA : public core::ResourceLoader
{
public:
	ImageLoaderTGA();
	const string& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	bool LoadResource(io::File* file, core::Resource* dst);
	bool LoadImageFormat(io::File* file, math::dimension2du& outSize, video::ColorFormat& outFormat);
	bool LoadImageToMemory(io::File* file, void* dest);
	void CleanUp();
	bool LoadTgaInfo(io::File* file, bool silent);

private:
	tga_struct* m_Tga;
	tga_info* m_Info;
	io::File* m_CurFile;
};

}
}

#endif // #ifndef INCLUDED_IMAGELOADER_TGA