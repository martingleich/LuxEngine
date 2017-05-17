#include "scene/mesh/GeometryCreatorLibImpl.h"
#include "core/ParamPackage.h"
#include "video/VideoDriver.h"
#include "video/SubMesh.h"
#include "scene/mesh/Mesh.h"

#include "scene/mesh/GeometryCreatorPlane.h"
#include "scene/mesh/GeometryCreatorSphereUV.h"
#include "scene/mesh/GeometryCreatorCube.h"
#include "scene/mesh/GeometryCreatorArrow.h"

namespace lux
{
namespace scene
{

GeometryCreatorLibImpl::GeometryCreatorLibImpl(video::VideoDriver* driver, scene::MeshSystem* meshCache, core::ResourceSystem* resSys) :
	m_Driver(driver),
	m_MeshSystem(meshCache),
	m_ResourceSystem(resSys)
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

StrongRef<video::SubMesh> GeometryCreatorLibImpl::CreateSubMesh(const string& name, const core::PackagePuffer& params)
{
	return GetByName(name)->CreateSubMesh(m_Driver, params);
}

StrongRef<scene::Mesh> GeometryCreatorLibImpl::CreateMesh(const string& name, const core::PackagePuffer& params)
{
	StrongRef<video::SubMesh> sub = GetByName(name)->CreateSubMesh(m_Driver, params);
	StrongRef<scene::Mesh> out = m_MeshSystem->CreateMesh();
	out->AddSubMesh(sub);
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<scene::Mesh> GeometryCreatorLibImpl::CreatePlaneMesh(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	StrongRef<GeometryCreatorPlane> creator = m_PlaneCreator;

	StrongRef<video::SubMesh> sub = creator->CreateSubMesh(m_Driver, sizeX, sizeY, tesX, tesY, texX, texY, function, context);
	StrongRef<scene::Mesh> out = m_MeshSystem->CreateMesh();
	out->AddSubMesh(sub);
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<scene::Mesh> GeometryCreatorLibImpl::CreateSphereMesh(
	float radius,
	s32 rings, s32 segments,
	float texX, float texY,
	bool inside)
{
	StrongRef<GeometryCreatorSphereUV> creator = m_SphereUVCreator;
	StrongRef<video::SubMesh> sub = creator->CreateSubMesh(m_Driver, radius, rings, segments, texX, texY, inside);
	StrongRef<scene::Mesh> out = m_MeshSystem->CreateMesh();
	out->AddSubMesh(sub);
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<scene::Mesh> GeometryCreatorLibImpl::CreateCubeMesh(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	StrongRef<GeometryCreatorCube> creator = m_CubeGenerator;
	StrongRef<video::SubMesh> sub = creator->CreateSubMesh(
		m_Driver,
		sizeX, sizeY, sizeZ,
		tesX, tesY, tesZ,
		texX, texY, texZ,
		inside);
	StrongRef<scene::Mesh> out = m_MeshSystem->CreateMesh();
	out->AddSubMesh(sub);
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<scene::Mesh> GeometryCreatorLibImpl::CreateArrowMesh(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	StrongRef<GeometryCreatorArrow> creator = m_ArrowCreator;
	StrongRef<video::SubMesh> sub = creator->CreateSubMesh(
		m_Driver,
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
	StrongRef<scene::Mesh> out = m_MeshSystem->CreateMesh();
	out->AddSubMesh(sub);
	out->RecalculateBoundingBox();

	return out;
}

}
}
