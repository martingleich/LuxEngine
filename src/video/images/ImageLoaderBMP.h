#ifndef INCLUDED_IMAGELOADERBMP_H
#define INCLUDED_IMAGELOADERBMP_H
#include "resources/ResourceLoader.h"

//#define USE_ALPHA_CHANNEL

namespace lux
{
namespace video
{

class ImageLoaderBMP : public core::ResourceLoader
{
public:
	bool LoadResource(io::File* file, core::Resource* dst);
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	const string& GetName() const;
};

}
}

#endif // !INCLUDED_CIMAGELOADERBMP_H
