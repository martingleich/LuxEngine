#ifndef INCLUDED_LUX_IMAGE_LOADER_PNG_H
#define INCLUDED_LUX_IMAGE_LOADER_PNG_H
#include "core/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderPNG : public core::ResourceLoader
{
public:
	const core::String& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Referable* dst);
};

}
}

#endif // !INCLUDED_LUX_IMAGE_LOADER_PNG_H