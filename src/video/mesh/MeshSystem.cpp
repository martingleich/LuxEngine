#include "video/mesh/MeshSystem.h"
#include "video/mesh/StaticMesh.h"

#include "core/ReferableFactory.h"

#include "video/VideoDriver.h"
#include "video/mesh/MeshLoaderOBJ.h"

#include "video/mesh/GeometryCreatorPlane.h"
#include "video/mesh/GeometryCreatorSphereUV.h"
#include "video/mesh/GeometryCreatorCube.h"
#include "video/mesh/GeometryCreatorArrow.h"

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

	core::ResourceSystem::Instance()->AddResourceLoader(LUX_NEW(MeshLoaderOBJ)(VideoDriver::Instance(), m_MatLib));
	core::ReferableFactory::Instance()->RegisterType(LUX_NEW(StaticMesh));

	m_PlaneCreator = LUX_NEW(GeometryCreatorPlane);
	AddCreator(m_PlaneCreator);
	m_SphereUVCreator = LUX_NEW(GeometryCreatorSphereUV);
	AddCreator(m_SphereUVCreator);
	m_CubeGenerator = LUX_NEW(GeometryCreatorCube);
	AddCreator(m_CubeGenerator);
	m_ArrowCreator = LUX_NEW(GeometryCreatorArrow);
	AddCreator(m_ArrowCreator);
}

MeshSystem::~MeshSystem()
{
}

StrongRef<Mesh> MeshSystem::CreateMesh()
{
	return core::ReferableFactory::Instance()->Create(ReferableType::Resource, core::ResourceType::Mesh);
}

StrongRef<GeometryCreator> MeshSystem::GetCreatorByName(const string& name) const
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return *it;
}

void MeshSystem::AddCreator(GeometryCreator* creator)
{
	LX_CHECK_NULL_ARG(creator);

	auto it = m_Creators.Find(creator->GetName());
	if(it != m_Creators.End())
		throw core::ErrorException("Geometry creator alredy exists");

	m_Creators.Set(creator->GetName(), creator);
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

core::PackagePuffer MeshSystem::GetCreatorParams(const string& name)
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name.Data());

	return core::PackagePuffer(&((*it)->GetParams()));
}

StrongRef<Geometry> MeshSystem::CreateGeometry(const string& name, const core::PackagePuffer& params)
{
	return GetCreatorByName(name)->CreateGeometry(params);
}

StrongRef<Mesh> MeshSystem::CreateMesh(const string& name, const core::PackagePuffer& params)
{
	StrongRef<Geometry> sub = GetCreatorByName(name)->CreateGeometry(params);
	StrongRef<Mesh> out = CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> MeshSystem::CreatePlaneMesh(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	StrongRef<GeometryCreatorPlane> creator = m_PlaneCreator;

	StrongRef<Geometry> sub = creator->CreateGeometry(sizeX, sizeY, tesX, tesY, texX, texY, function, context);
	StrongRef<Mesh> out = CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> MeshSystem::CreateSphereMesh(
	float radius,
	s32 rings, s32 sectors,
	float texX, float texY,
	bool inside)
{
	StrongRef<GeometryCreatorSphereUV> creator = m_SphereUVCreator;
	StrongRef<Geometry> sub = creator->CreateGeometry(radius, rings, sectors, texX, texY, inside);
	StrongRef<Mesh> out = CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> MeshSystem::CreateCubeMesh(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	StrongRef<GeometryCreatorCube> creator = m_CubeGenerator;
	StrongRef<Geometry> sub = creator->CreateGeometry(
		sizeX, sizeY, sizeZ,
		tesX, tesY, tesZ,
		texX, texY, texZ,
		inside);
	StrongRef<Mesh> out = CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

StrongRef<Mesh> MeshSystem::CreateArrowMesh(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	StrongRef<GeometryCreatorArrow> creator = m_ArrowCreator;
	StrongRef<Geometry> sub = creator->CreateGeometry(
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
	StrongRef<Mesh> out = CreateMesh();
	out->AddGeometry(sub);
	out->SetMaterial(0, m_MatLib->CreateMaterial());
	out->RecalculateBoundingBox();

	return out;
}

} // namespace scene
} // namespace lux
