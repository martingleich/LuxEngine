#ifndef INCLUDED_MESH_LOADER_X_H
#define INCLUDED_MESH_LOADER_X_H
#include "core/ResourceSystem.h"

namespace lux
{
namespace scene
{
class Armature;
}
namespace video
{
class Mesh;
class MeshLoaderX : public core::ResourceLoader
{
public:
	MeshLoaderX();

	core::Name GetResourceType(io::File* file, core::Name requestedType = core::Name::INVALID);
	void LoadResource(io::File* file, core::Resource* dst);
	void LoadMesh(io::File* file, video::Mesh* dst, const core::String& meshToLoad = core::String::EMPTY, bool mergeMeshes = true);
#if 0
	void LoadArmature(io::File* file, scene::Armature* dst);
#endif
	const core::String& GetName() const;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_MESH_LOADER_X_H