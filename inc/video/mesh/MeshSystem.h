#ifndef INCLUDED_LUX_MESH_SYSTEM_H
#define INCLUDED_LUX_MESH_SYSTEM_H
#include "core/ReferenceCounted.h"

namespace lux
{
namespace video
{
class Mesh;
class Geometry;
class Material;

//! Class todo mesh related things
class MeshSystem : public ReferenceCounted, core::Uncopyable
{
public:
	LUX_API MeshSystem();
	LUX_API ~MeshSystem();

	LUX_API static void Initialize();
	LUX_API static MeshSystem* Instance();
	LUX_API static void Destroy();

	LUX_API StrongRef<Mesh> CreateEmptyMesh();
	LUX_API StrongRef<Mesh> CreateMeshDefaultMaterial(Geometry* geo);
	LUX_API StrongRef<Mesh> CreateMesh(Geometry* geo, Material* mat);
};

} // namespace scene
} // namespace lux

#endif