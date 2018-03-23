#ifndef INCLUDED_LUX_IMAGELOADER_TGA_H
#define INCLUDED_LUX_IMAGELOADER_TGA_H
#include "core/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderTGA : public core::ResourceLoader
{
public:
	const core::String& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
};

}
}

#endif // #ifndef INCLUDED_LUX_IMAGELOADER_TGA_H