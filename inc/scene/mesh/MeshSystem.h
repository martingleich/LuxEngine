#ifndef INCLUDED_MESHCACHE_H
#define INCLUDED_MESHCACHE_H
#include "io/path.h"
#include "core/ReferenceCounted.h"

namespace lux
{
namespace io
{
class File;
}
namespace scene
{
class Mesh;
class GeometryCreatorLib;

//! Loads and caches meshes by name, to prevent unnecessary reloads
class MeshSystem : public ReferenceCounted
{
public:
	virtual ~MeshSystem() {}

	//! Add new mesh to the list
	/**
	\param name The name of the new mesh
	*/
	virtual StrongRef<Mesh> AddMesh(const io::path& name) = 0;

	//! Create a new unmanaged mesh
	/**
	\return The new mesh
	*/
	virtual StrongRef<Mesh> CreateMesh() = 0;

	//! Get the geometry creator library.
	virtual StrongRef<GeometryCreatorLib> GetGeometryCreatorLib() = 0;
};

} // namespace scene
} // namespace lux

#endif