#include "MeshSystemImpl.h"
#include "GeometryCreatorLibImpl.h"
#include "StaticMesh.h"

#include "video/VideoDriver.h"

#include "scene/mesh/MeshLoaderOBJ.h"
#include "core/ReferableFactory.h"

namespace lux
{
namespace scene
{

MeshSystemImpl::MeshSystemImpl(core::ResourceSystem* resourceSystem,video::VideoDriver* driver, video::MaterialLibrary* matLib) :
	m_ResourceSystem(resourceSystem),
	m_VideoDriver(driver),
	m_MatLib(matLib)
{
	m_GeoCreatorLib = LUX_NEW(GeometryCreatorLibImpl)(m_MatLib, m_VideoDriver, this, m_ResourceSystem);

	m_ResourceSystem->AddResourceLoader(LUX_NEW(MeshLoaderOBJ)(m_VideoDriver, m_MatLib, m_ResourceSystem));

	m_ResourceSystem->GetReferableFactory()->RegisterType(LUX_NEW(StaticMesh)(m_VideoDriver));
}

StrongRef<Mesh> MeshSystemImpl::AddMesh(const io::path& name)
{
	auto mesh = CreateMesh();
	m_ResourceSystem->AddResource(name, mesh);
	return mesh;
}

StrongRef<Mesh> MeshSystemImpl::CreateMesh()
{
	return m_ResourceSystem->GetReferableFactory()->Create(ReferableType::Resource, core::ResourceType::Mesh);
}

StrongRef<GeometryCreatorLib> MeshSystemImpl::GetGeometryCreatorLib()
{
	return m_GeoCreatorLib;
}

} // namespace scene
} // namespace lux
