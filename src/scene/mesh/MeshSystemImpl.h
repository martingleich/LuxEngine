#ifndef INCLUDED_CMESHCACHE_H
#define INCLUDED_CMESHCACHE_H
#include "scene/mesh/MeshSystem.h"

namespace lux
{
namespace core
{
class ResourceSystem;
}
namespace video
{
class VideoDriver;
class MaterialLibrary;
}
namespace scene
{

class MeshSystemImpl : public MeshSystem
{
public:
	MeshSystemImpl(core::ResourceSystem* resourceSystem, video::VideoDriver* driver, video::MaterialLibrary* matLib);

	StrongRef<Mesh> AddMesh(const io::path& name);
	StrongRef<Mesh> CreateMesh();

	StrongRef<GeometryCreatorLib> GetGeometryCreatorLib();

private:
	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<video::VideoDriver> m_VideoDriver;
	StrongRef<GeometryCreatorLib> m_GeoCreatorLib;
	StrongRef<video::MaterialLibrary> m_MatLib;
};

}    

}    


#endif