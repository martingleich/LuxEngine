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
	void AddMesh(const io::path& name, Mesh* mesh);
	StrongRef<Mesh> GetMesh(const io::path& filename);
	StrongRef<Mesh> GetMesh(io::File* file);
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