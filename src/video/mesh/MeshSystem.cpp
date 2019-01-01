#include "video/mesh/MeshSystem.h"

#include "core/ReferableFactory.h"

#include "video/VideoDriver.h"
#include "video/Material.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"

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

void MeshSystem::Initialize(Material* defaultMaterial)
{
	g_MeshSystem = LUX_NEW(MeshSystem)(defaultMaterial);
}

MeshSystem* MeshSystem::Instance()
{
	return g_MeshSystem;
}

void MeshSystem::Destroy()
{
	g_MeshSystem.Reset();
}

MeshSystem::MeshSystem(Material* defaultMaterial)
{
	LX_CHECK_NULL_ARG(defaultMaterial);

	m_DefaultMaterial = defaultMaterial;

	AddCreator("plane", LUX_NEW(GeometryCreatorPlane));
	AddCreator("sphereUV", LUX_NEW(GeometryCreatorSphereUV));
	AddCreator("cube", LUX_NEW(GeometryCreatorCube));
	AddCreator("arrow", LUX_NEW(GeometryCreatorArrow));
	AddCreator("torus", LUX_NEW(GeometryCreatorTorus));
	AddCreator("cylinder", LUX_NEW(GeometryCreatorCylinder));
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

StrongRef<GeometryCreator> MeshSystem::GetCreatorByName(core::StringView name) const
{
	auto it = m_Creators.Find(name);
	if(it == m_Creators.End())
		throw core::ObjectNotFoundException(name);

	return *it;
}

StrongRef<GeometryCreator> MeshSystem::AddCreator(core::StringView name, GeometryCreator* creator)
{
	LX_CHECK_NULL_ARG(creator);

	auto it = m_Creators.Find(name);
	if(it != m_Creators.End())
		throw core::InvalidOperationException("Geometry creator already exists");
	m_Creators.Set(name, creator);

	return creator;
}

void MeshSystem::RemoveCreator(core::StringView name)
{
	m_Creators.Erase(name);
}

core::AnyRange<core::String> MeshSystem::GetCreatorNames() const
{
	auto keys = m_Creators.Keys();
	return core::MakeAnyRange(keys.First(), keys.End());
}

StrongRef<Geometry> MeshSystem::CreateGeometry(core::StringView name, const core::PackagePuffer& params)
{
	return GetCreatorByName(name)->CreateGeometry(params);
}

StrongRef<Mesh> MeshSystem::CreateMesh(core::StringView name, const core::PackagePuffer& params)
{
	StrongRef<Geometry> sub = GetCreatorByName(name)->CreateGeometry(params);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreatePlaneMesh(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	auto creator = GetCreatorByName("plane").As<GeometryCreatorPlane>();

	StrongRef<Geometry> sub = creator->CreateGeometry(
		sizeX, sizeY,
		tesX, tesY,
		texX, texY,
		function, context);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreateSphereMesh(
	float radius,
	s32 rings, s32 sectors,
	float texX, float texY,
	bool inside)
{
	auto creator = GetCreatorByName("sphereUV").As<GeometryCreatorSphereUV>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radius,
		rings, sectors,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreateCubeMesh(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	auto creator = GetCreatorByName("cube").As<GeometryCreatorCube>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		sizeX, sizeY, sizeZ,
		tesX, tesY, tesZ,
		texX, texY, texZ,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreateArrowMesh(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	auto creator = GetCreatorByName("arrow").As<GeometryCreatorArrow>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreateCylinderMesh(
	float radius, float height,
	s32 sectors, s32 planes,
	s32 texX, s32 texY,
	bool inside)
{
	auto creator = GetCreatorByName("cylinder").As<GeometryCreatorCylinder>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radius, height,
		sectors, planes,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

StrongRef<Mesh> MeshSystem::CreateTorusMesh(
	float radiusMajor, float radiusMinor,
	s32 sectorsMajor, s32 sectorsMinor,
	s32 texX, s32 texY,
	bool inside)
{
	auto creator = GetCreatorByName("torus").As<GeometryCreatorTorus>();
	StrongRef<Geometry> sub = creator->CreateGeometry(
		radiusMajor, radiusMinor,
		sectorsMajor, sectorsMinor,
		texX, texY,
		inside);
	StrongRef<Mesh> out = CreateMesh(sub);
	out->SetMaterial(m_DefaultMaterial->Clone());

	return out;
}

} // namespace scene
} // namespace lux
