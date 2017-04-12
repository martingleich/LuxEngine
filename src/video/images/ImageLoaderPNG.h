#pragma once
#ifndef INCLUDED_IMAGE_LOADER_PNG_H
#define INCLUDED_IMAGE_LOADER_PNG_H
#include "resources/ResourceLoader.h"
#include "math/dimension2d.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

class ImageLoaderPNG : public core::ResourceLoader
{
public:
	const string& GetName() const;
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	bool LoadResource(io::File* file, core::Resource* dst);
};

}
}

#endif // !INCLUDED_IMAGE_LOADER_PNG_H