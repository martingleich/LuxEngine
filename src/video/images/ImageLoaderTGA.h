#ifndef INCLUDED_IMAGELOADER_TGA_H
#define INCLUDED_IMAGELOADER_TGA_H
#include "resources/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderTGA : public core::ResourceLoader
{
public:
	const String& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
};

}
}

#endif // #ifndef INCLUDED_IMAGELOADER_TGA_H