#ifndef INCLUDED_OBJ_MESHLOADER_H
#define INCLUDED_OBJ_MESHLOADER_H
#include "resources/ResourceLoader.h"

namespace lux
{
namespace video
{

class MeshLoaderOBJ : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
	const String& GetName() const;
};

}
}

#endif
