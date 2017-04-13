#include "MeshSystemImpl.h"
#include "GeometryCreatorLibImpl.h"
#include "StaticMesh.h"

#include "video/VideoDriver.h"

#include "scene/mesh/MeshLoaderOBJ.h"

namespace lux
{
namespace scene
{

MeshSystemImpl::MeshSystemImpl(core::ResourceSystem* resourceSystem, video::VideoDriver* driver, video::MaterialLibrary* matLib) :
	m_ResourceSystem(resourceSystem),
	m_VideoDriver(driver),
	m_MatLib(matLib)
{
	m_GeoCreatorLib = LUX_NEW(GeometryCreatorLibImpl)(m_VideoDriver, this, m_ResourceSystem);

	m_ResourceSystem->AddResourceLoader(LUX_NEW(MeshLoaderOBJ)(m_VideoDriver, m_MatLib, m_ResourceSystem));

	m_ResourceSystem->GetReferableFactory()->RegisterType(LUX_NEW(StaticMesh));
}


bool MeshSystemImpl::AddMesh(const io::path& name, Mesh* mesh)
{
	return m_ResourceSystem->AddResource(name, mesh);
}

StrongRef<Mesh> MeshSystemImpl::CreateMesh()
{
	return LUX_NEW(StaticMesh);
}

StrongRef<Mesh> MeshSystemImpl::GetMesh(const io::path& filename)
{
	return m_ResourceSystem->GetResource(core::ResourceType::Mesh, filename);
}

StrongRef<Mesh> MeshSystemImpl::GetMesh(io::File* file)
{
	return m_ResourceSystem->GetResource(core::ResourceType::Mesh, file);
}

StrongRef<GeometryCreatorLib> MeshSystemImpl::GetGeometryCreatorLib()
{
	return m_GeoCreatorLib;
}

}    

}    

