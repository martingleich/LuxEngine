#ifndef INCLUDED_IMESHCACHE_H
#define INCLUDED_IMESHCACHE_H
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
class MeshCache : public ReferenceCounted
{
public:
	virtual ~MeshCache()
	{
	}

	//! Add new mesh to the list
	/**
	\param name The name of the new mesh, if the name is empty the mesh is not cached
	\param mesh The mesh to add to the list
	*/
	virtual bool AddMesh(const io::path& name, Mesh* mesh) = 0;

	//! Retrieves a model from cache or loads it if necessary
	/**
	\param filename The filename of the mesh
	\return The loaded mesh
	*/
	virtual StrongRef<Mesh> GetMesh(const io::path& filename) = 0;

	//! Retrieves a model from cache or loads it if necessary
	/**
	\param filename The file which contains the mesh
	\return The loaded mesh
	*/
	virtual StrongRef<Mesh> GetMesh(io::File* file) = 0;

	//! Create a new unmanaged mesh
	/**
	\return The new mesh
	*/
	virtual StrongRef<Mesh> CreateMesh() = 0;

	//! Get the geometry creator library.
	virtual StrongRef<GeometryCreatorLib> GetGeometryCreatorLib() = 0;
};

}    // namespace scene
}    // namespace lux

#endif