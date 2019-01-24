#ifndef INCLUDED_LUX_IMAGELOADERPNM_H
#define INCLUDED_LUX_IMAGELOADERPNM_H
#include "core/ResourceLoader.h"

namespace lux
{
namespace video
{

class ImageLoaderPNM : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const core::String& GetName() const;
	void LoadResource(io::File* file, core::Referable* dst);
};

}
}

#endif