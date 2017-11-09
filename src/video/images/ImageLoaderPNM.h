#ifndef INCLUDED_IMAGELOADERPNM_H
#define INCLUDED_IMAGELOADERPNM_H
#include "resources/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderPNM : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const core::String& GetName() const;
	void LoadResource(io::File* file, core::Resource* dst);
};

}
}

#endif