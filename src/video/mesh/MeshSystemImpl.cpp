#include "video/mesh/MeshSystemImpl.h"
#include "video/mesh/GeometryCreatorLibImpl.h"
#include "video/mesh/StaticMesh.h"

#include "video/VideoDriver.h"

#include "video/mesh/MeshLoaderOBJ.h"
#include "core/ReferableFactory.h"

namespace lux
{
namespace video
{

MeshSystemImpl::MeshSystemImpl(video::VideoDriver* driver, video::MaterialLibrary* matLib) :
	m_VideoDriver(driver),
	m_MatLib(matLib)
{
	m_GeoCreatorLib = LUX_NEW(GeometryCreatorLibImpl)(m_MatLib, m_VideoDriver, this);

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(MeshLoaderOBJ)(m_VideoDriver, m_MatLib));
	core::ReferableFactory::Instance()->RegisterType(LUX_NEW(StaticMesh)(m_VideoDriver));
}

StrongRef<Mesh> MeshSystemImpl::AddMesh(const io::path& name)
{
	auto mesh = CreateMesh();
	core::ResourceSystem::Instance()->AddResource(name, mesh);
	return mesh;
}

StrongRef<Mesh> MeshSystemImpl::CreateMesh()
{
	return core::ReferableFactory::Instance()->Create(ReferableType::Resource, core::ResourceType::Mesh);
}

StrongRef<GeometryCreatorLib> MeshSystemImpl::GetGeometryCreatorLib()
{
	return m_GeoCreatorLib;
}

} // namespace scene
} // namespace lux
