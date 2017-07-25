#ifndef INCLUDED_IMAGE_LOADER_PNG_H
#define INCLUDED_IMAGE_LOADER_PNG_H
#include "resources/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderPNG : public core::ResourceLoader
{
public:
	const String& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
};

}
}

#endif // !INCLUDED_IMAGE_LOADER_PNG_H