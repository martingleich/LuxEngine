#ifndef INCLUDED_CMESHCACHE_H
#define INCLUDED_CMESHCACHE_H
#include "scene/mesh/MeshCache.h"

namespace lux
{
namespace core
{
class ResourceSystem;
}
namespace video
{
class VideoDriver;
}
namespace scene
{

class MeshCacheImpl : public MeshCache
{
public:
	MeshCacheImpl(core::ResourceSystem* resourceSystem, video::VideoDriver* driver);
	bool AddMesh(const io::path& name, Mesh* mesh);
	StrongRef<Mesh> GetMesh(const io::path& filename);
	StrongRef<Mesh> GetMesh(io::File* file);
	StrongRef<Mesh> CreateMesh();

	StrongRef<GeometryCreatorLib> GetGeometryCreatorLib();

private:
	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<video::VideoDriver> m_VideoDriver;
	StrongRef<GeometryCreatorLib> m_GeoCreatorLib;
};

}    // namespace scene
}    // namespace lux

#endif