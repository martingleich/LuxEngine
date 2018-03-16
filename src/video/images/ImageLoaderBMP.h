#ifndef INCLUDED_IMAGELOADERBMP_H
#define INCLUDED_IMAGELOADERBMP_H
#include "core/ResourceLoader.h"

//#define USE_ALPHA_CHANNEL

namespace lux
{
namespace video
{

class ImageLoaderBMP : public core::ResourceLoader
{
public:
	void LoadResource(io::File* file, core::Resource* dst);
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const core::String& GetName() const;
};

}
}

#endif // !INCLUDED_CIMAGELOADERBMP_H
