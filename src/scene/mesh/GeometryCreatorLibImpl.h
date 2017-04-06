#ifndef INCLUDED_GEOMETRY_CREATOR_LIB_IMPL_H
#define INCLUDED_GEOMETRY_CREATOR_LIB_IMPL_H
#include "scene/mesh/GeometryCreatorLib.h"
#include "core/lxhashmap.h"

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

class GeometryCreatorLibImpl : public GeometryCreatorLib
{
public:
	GeometryCreatorLibImpl(video::VideoDriver* driver, scene::MeshCache* meshCache, core::ResourceSystem* resSystem);
	StrongRef<GeometryCreator> GetByName(const string& name) const;
	void AddCreator(GeometryCreator* creator);
	void RemoveCreator(GeometryCreator* creator);
	size_t GetCreatorCount() const;
	StrongRef<GeometryCreator> GetById(size_t id) const;

	core::PackagePuffer GetCreatorParams(const string& name);
	StrongRef<scene::Mesh> CreateMesh(const string& name, const core::PackagePuffer& params);
	StrongRef<video::SubMesh> CreateSubMesh(const string& name, const core::PackagePuffer& params);

	StrongRef<scene::Mesh> CreatePlaneMesh(float sizeX, float sizeY, s32 tesX, s32 tesY, float texX, float texY, float(*function)(void* ctx, float x, float y), void* context);
	StrongRef<scene::Mesh> CreateSphereMesh(float radius, s32 rings, s32 segments, float texX, float texY, bool inside);
	StrongRef<scene::Mesh> CreateCubeMesh(
		float sizeX, float sizeY, float sizeZ,
		s32 tesX, s32 tesY, s32 tesZ,
		float texX, float texY, float texZ,
		bool inside);

	StrongRef<scene::Mesh> CreateArrowMesh(
		float shaft_height, float head_height,
		float shaft_radius, float head_radius,
		s32 sectors);

private:
	struct Entry
	{
		StrongRef<GeometryCreator> creator;
	};

private:
	core::HashMap<string, Entry> m_Creators;
	WeakRef<video::VideoDriver> m_Driver;
	WeakRef<scene::MeshCache> m_MeshCache;
	WeakRef<core::ResourceSystem> m_ResourceSystem;

	StrongRef<GeometryCreator> m_PlaneCreator;
	StrongRef<GeometryCreator> m_SphereUVCreator;
	StrongRef<GeometryCreator> m_ArrowCreator;
	StrongRef<GeometryCreator> m_CubeGenerator;
};

}
}

#endif // #ifndef INCLUDED_GEOMETRY_CREATOR_LIB_IMPL_H