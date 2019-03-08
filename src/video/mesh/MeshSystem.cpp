#include "video/mesh/MeshSystem.h"

#include "video/Material.h"
#include "video/MaterialLibrary.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

static StrongRef<MeshSystem> g_MeshSystem;

void MeshSystem::Initialize()
{
	g_MeshSystem = LUX_NEW(MeshSystem);
}

MeshSystem* MeshSystem::Instance()
{
	return g_MeshSystem;
}

void MeshSystem::Destroy()
{
	g_MeshSystem.Reset();
}

MeshSystem::MeshSystem()
{
}

MeshSystem::~MeshSystem()
{
}

StrongRef<Mesh> MeshSystem::CreateMesh(Geometry* geo, Material* mat)
{
	LX_CHECK_NULL_ARG(mat);

	return LUX_NEW(Mesh)(geo, mat->Clone());
}

StrongRef<Mesh> MeshSystem::CreateMeshDefaultMaterial(Geometry* geo)
{
	auto defMat = video::MaterialLibrary::Instance()->GetMaterial(video::MaterialLibrary::SolidName);
	return CreateMesh(geo, defMat);
}

} // namespace scene
} // namespace lux
