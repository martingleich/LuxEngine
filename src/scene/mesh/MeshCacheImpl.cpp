#include "MeshCacheImpl.h"
#include "GeometryCreatorLibImpl.h"
#include "StaticMesh.h"

#include "video/VideoDriver.h"

namespace lux
{
namespace scene
{

MeshCacheImpl::MeshCacheImpl(core::ResourceSystem* resourceSystem, video::VideoDriver* driver) :
	m_ResourceSystem(resourceSystem),
	m_VideoDriver(driver)
{
	m_GeoCreatorLib = LUX_NEW(GeometryCreatorLibImpl)(driver, this, resourceSystem);

	resourceSystem->GetReferableFactor()->RegisterType(LUX_NEW(StaticMesh));
}


bool MeshCacheImpl::AddMesh(const io::path& name, Mesh* mesh)
{
	return m_ResourceSystem->AddResource(name, mesh);
}

StrongRef<Mesh> MeshCacheImpl::CreateMesh()
{
	return LUX_NEW(StaticMesh);
}

StrongRef<Mesh> MeshCacheImpl::GetMesh(const io::path& filename)
{
	return m_ResourceSystem->GetResource(core::ResourceType::Mesh, filename);
}

StrongRef<Mesh> MeshCacheImpl::GetMesh(io::File* file)
{
	return m_ResourceSystem->GetResource(core::ResourceType::Mesh, file);
}

StrongRef<GeometryCreatorLib> MeshCacheImpl::GetGeometryCreatorLib()
{
	return m_GeoCreatorLib;
}

}    

}    

