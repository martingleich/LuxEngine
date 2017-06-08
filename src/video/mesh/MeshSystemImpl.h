#ifndef INCLUDED_CMESHCACHE_H
#define INCLUDED_CMESHCACHE_H
#include "video/mesh/MeshSystem.h"

namespace lux
{
namespace video
{
class VideoDriver;
class MaterialLibrary;
}
namespace video
{

class MeshSystemImpl : public MeshSystem
{
public:
	MeshSystemImpl(video::VideoDriver* driver, video::MaterialLibrary* matLib);

	StrongRef<Mesh> AddMesh(const io::path& name);
	StrongRef<Mesh> CreateMesh();

	StrongRef<GeometryCreatorLib> GetGeometryCreatorLib();

private:
	StrongRef<video::VideoDriver> m_VideoDriver;
	StrongRef<GeometryCreatorLib> m_GeoCreatorLib;
	StrongRef<video::MaterialLibrary> m_MatLib;
};

} // namespace scene
} // namespace lux

#endif