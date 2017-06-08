#include "video/mesh/GeometryCreatorLibImpl.h"
#include "core/ParamPackage.h"
#include "video/VideoDriver.h"
#include "video/mesh/Geometry.h"
#include "video/MaterialLibrary.h"
#include "video/Material.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/MeshSystem.h"

#include "resources/ResourceSystem.h"

#include "video/mesh/GeometryCreatorPlane.h"
#include "video/mesh/GeometryCreatorSphereUV.h"
#include "video/mesh/GeometryCreatorCube.h"
#include "video/mesh/GeometryCreatorArrow.h"

namespace lux
{
namespace video
{

GeometryCreatorLibImpl::GeometryCreatorLibImpl(MaterialLibrary* matLib, VideoDriver* driver, MeshSystem* meshCache, core::ResourceSystem* resSys) :
	m_Driver(driver),
	m_MeshSystem(meshCache),
	m_ResourceSystem(resSys),
	m_MatLib(matLib)
{
	m_PlaneCreator = LUX_NEW(GeometryCreatorPlane);
	AddCreator(m_PlaneCreator);
	m_SphereUVCreator = LUX_NEW(GeometryCreatorSphereUV);
	AddCreator(m_SphereUVCreator);
	m_CubeGenerator = LUX_NEW(GeometryCreatorCube);
	AddCreator(m_CubeGenerator);
	m_ArrowCreator = LUX_NEW(GeometryCreatorArrow);
	AddCreator(m_ArrowCreator);
}

StrongRef<GeometryCreator> GeometryCreatorLibImpl::GetByName(const string& name) const
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return it->creator;
}

void GeometryCreatorLibImpl::AddCreator(GeometryCreator* creator)
{
	LX_CHECK_NULL_ARG(creator);

	auto it = m_Creators.Find(creator->GetName());
	if(it != m_Creators.End())
		throw core::ErrorException("Geometry creator alredy exists");

	Entry entry;
	entry.creator = creator;
	m_Creators.Set(creator->GetName(), entry);
}

void GeometryCreatorLibImpl::RemoveCreator(GeometryCreator* creator)
{
	if(creator)
		m_Creators.Erase(creator->GetName());
}

size_t GeometryCreatorLibImpl::GetCreatorCount() const
{
	return m_Creators.Size();
}

StrongRef<GeometryCreator> GeometryCreatorLibImpl::GetById(size_t id) const
{
	if(id >= m_Creators.Size())
		throw core::OutOfRangeException();

	return core::AdvanceIterator(m_Creators.First(), id)->creator;
}

core::PackagePuffer GeometryCreatorLibImpl::GetCreatorParams(const string& name)
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return core::PackagePuffer(&it->creator->GetParams());
}

StrongRef<Geometry> GeometryCreatorLibImpl::CreateGeometry(const string& name, const core::PackagePuffer& params)
{
	return GetByName(name)->CreateGeometry(m_Driver, params);
}

StrongRef<Mesh> GeometryCreatorLibImpl::CreateMesh(const string& name, const core::PackagePuffer& params)
{
	StrongRef<Geometry> sub = GetByName(name)->CreateGeometry(m_Driver, params);
	StrongRef<Mesh> out = m_MeshSystem->CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> GeometryCreatorLibImpl::CreatePlaneMesh(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	StrongRef<GeometryCreatorPlane> creator = m_PlaneCreator;

	StrongRef<Geometry> sub = creator->CreateGeometry(m_Driver, sizeX, sizeY, tesX, tesY, texX, texY, function, context);
	StrongRef<Mesh> out = m_MeshSystem->CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> GeometryCreatorLibImpl::CreateSphereMesh(
	float radius,
	s32 rings, s32 sectors,
	float texX, float texY,
	bool inside)
{
	StrongRef<GeometryCreatorSphereUV> creator = m_SphereUVCreator;
	StrongRef<Geometry> sub = creator->CreateGeometry(m_Driver, radius, rings, sectors, texX, texY, inside);
	StrongRef<Mesh> out = m_MeshSystem->CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> GeometryCreatorLibImpl::CreateCubeMesh(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	StrongRef<GeometryCreatorCube> creator = m_CubeGenerator;
	StrongRef<Geometry> sub = creator->CreateGeometry(
		m_Driver,
		sizeX, sizeY, sizeZ,
		tesX, tesY, tesZ,
		texX, texY, texZ,
		inside);
	StrongRef<Mesh> out = m_MeshSystem->CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> GeometryCreatorLibImpl::CreateArrowMesh(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	StrongRef<GeometryCreatorArrow> creator = m_ArrowCreator;
	StrongRef<Geometry> sub = creator->CreateGeometry(
		m_Driver,
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
	StrongRef<Mesh> out = m_MeshSystem->CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

}
}
