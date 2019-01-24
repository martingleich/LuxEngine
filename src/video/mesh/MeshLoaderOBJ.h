#ifndef INCLUDED_LUX_OBJ_MESHLOADER_H
#define INCLUDED_LUX_OBJ_MESHLOADER_H
#include "core/ResourceLoader.h"

namespace lux
{
namespace video
{

class MeshLoaderOBJ : public core::ResourceLoader
{
public:
	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Referable* dst);
	const core::String& GetName() const;
};

}
}

#endif
