#include "video/mesh/MeshSystem.h"
#include "video/mesh/StaticMesh.h"

#include "core/ReferableFactory.h"
#include "resources/ResourceSystem.h"

#include "video/MaterialLibrary.h"
#include "video/VideoDriver.h"
#include "video/mesh/Geometry.h"

#include "video/mesh/MeshLoaderOBJ.h"

#include "video/mesh/GeometryCreatorPlane.h"
#include "video/mesh/GeometryCreatorSphereUV.h"
#include "video/mesh/GeometryCreatorCube.h"
#include "video/mesh/GeometryCreatorArrow.h"
#include "video/mesh/GeometryCreatorCylinder.h"
#include "video/mesh/GeometryCreatorTorus.h"

namespace lux
{
namespace video
{

static StrongRef<MeshSystem> g_MeshSystem;

void MeshSystem::Initialize(MeshSystem* system)
{
	if(!system)
		system = LUX_NEW(MeshSystem);

	if(!system)
		throw core::ErrorException("No mesh system available");
	g_MeshSystem = system;
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
	m_MatLib = MaterialLibrary::Instance();
	m_DefaultMaterial = m_MatLib->CloneMaterial("solid");

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(MeshLoaderOBJ));

	m_PlaneCreator = AddCreator(LUX_NEW(GeometryCreatorPlane));
	m_SphereUVCreator = AddCreator(LUX_NEW(GeometryCreatorSphereUV));
	m_CubeGenerator = AddCreator(LUX_NEW(GeometryCreatorCube));
	m_ArrowCreator = AddCreator(LUX_NEW(GeometryCreatorArrow));
	m_TorusGenerator = AddCreator(LUX_NEW(GeometryCreatorTorus));
	m_CylinderGenerator = AddCreator(LUX_NEW(GeometryCreatorCylinder));
}

MeshSystem::~MeshSystem()
{
}

StrongRef<Mesh> MeshSystem::CreateMesh()
{
	return core::ReferableFactory::Instance()->Create(
		core::ResourceType::Mesh).AsStrong<Mesh>();
}

StrongRef<Mesh> MeshSystem::CreateMesh(Geometry* geo)
{
	auto out = CreateMesh();
	out->SetGeometry(geo);
	out->RecalculateBoundingBox();
	return out;
}

StrongRef<GeometryCreator> MeshSystem::GetCreatorByName(
	const core::String& name) const
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return *it;
}

StrongRef<GeometryCreator> MeshSystem::AddCreator(GeometryCreator* creator)
{
	LX_CHECK_NULL_ARG(creator);

	auto it = m_Creators.Find(creator->GetName());
	if(it != m_Creators.End())
		throw core::ErrorException("Geometry creator already exists");

	m_Creators.Set(creator->GetName(), creator);

	return creator;
}

void MeshSystem::RemoveCreator(GeometryCreator* creator)
{
	LX_CHECK_NULL_ARG(creator);
	m_Creators.Erase(creator->GetName());
}

size_t MeshSystem::GetCreatorCount() const
{
	return m_Creators.Size();
}

StrongRef<GeometryCreator> MeshSystem::GetCreatorById(size_t id) const
{
	if(id >= m_Creators.Size())
		throw core::OutOfRangeException();

	return *core::AdvanceIterator(m_Creators.First(), id);
}

core::PackagePuffer MeshSystem::GetCreatorParams(const core::String& name)
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return core::PackagePuffer(&((*it)->GetParams()));
}

StrongRef<Geometry> MeshSystem::CreateGeometry(
	const core::String& name,
	const core::PackagePuffer& params)
{
	return GetCreatorByName(name)->CreateGeometry(params);
}

StrongRef<Mesh> MeshSystem::CreateMesh(
	const core::String& name,
	const core::PackagePuffer& params)
{
	StrongRef<Geometry> sub = GetCreatorByName(name)->CreateGeometry(params);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreatePlaneMesh(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	auto creator = m_PlaneCreator.StaticCastStrong<GeometryCreatorPlane>();

	StrongRef<Geometry> sub = creator->CreateGeometry(
		sizeX, sizeY,
		tesX, tesY,
		texX, texY,
		function, context);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreateSphereMesh(
	float radius,
	s32 rings, s32 sectors,
	float texX, float texY,
	bool inside)
{
	auto creator = m_SphereUVCreator.StaticCast<GeometryCreatorSphereUV>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radius,
		rings, sectors,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreateCubeMesh(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	auto creator = m_CubeGenerator.StaticCast<GeometryCreatorCube>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		sizeX, sizeY, sizeZ,
		tesX, tesY, tesZ,
		texX, texY, texZ,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreateArrowMesh(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	auto creator = m_ArrowCreator.StaticCastStrong<GeometryCreatorArrow>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreateCylinderMesh(
	float radius, float height,
	s32 sectors, s32 planes,
	s32 texX, s32 texY,
	bool inside)
{
	auto creator = m_CylinderGenerator.StaticCastStrong<GeometryCreatorCylinder>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radius, height,
		sectors, planes,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Mesh> MeshSystem::CreateTorusMesh(
	float radiusMajor, float radiusMinor,
	s32 sectorsMajor, s32 sectorsMinor,
	s32 texX, s32 texY,
	bool inside)
{
	auto creator = m_TorusGenerator.StaticCastStrong<GeometryCreatorTorus>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radiusMajor, radiusMinor,
		sectorsMajor, sectorsMinor,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial);

	return out;
}

StrongRef<Material> MeshSystem::GetDefaultMaterial()
{
	return m_DefaultMaterial;
}

void MeshSystem::SetDefaultMaterial(Material* m)
{
	m_DefaultMaterial = m;
}

} // namespace scene
} // namespace lux
