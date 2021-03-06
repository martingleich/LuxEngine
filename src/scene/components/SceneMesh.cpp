#include "scene/components/SceneMesh.h"
#include "scene/Node.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/Renderer.h"

/*
Improve handling of readOnlyMaterial.
*/
LX_REFERABLE_MEMBERS_SRC(lux::scene::Mesh, "lux.comp.Mesh");

namespace lux
{
namespace scene
{

static ERenderPass GetPassFromReq(video::EMaterialReqFlag req)
{
	if(TestFlag(req, video::EMaterialReqFlag::Transparent))
		return ERenderPass::Transparent;
	else
		return ERenderPass::Solid;
}

Mesh::Mesh() :
	m_OnlyReadMaterials(true)
{
}

Mesh::Mesh(video::Mesh* mesh) :
	m_OnlyReadMaterials(true)
{
	SetMesh(mesh);
}

Mesh::Mesh(const Mesh& other) :
	m_Mesh(other.m_Mesh),
	m_OnlyReadMaterials(other.m_OnlyReadMaterials),
	m_BoundingBox(other.m_BoundingBox)
{
	if(!m_OnlyReadMaterials) {
		for(auto it = other.m_Materials.First(); it != other.m_Materials.End(); ++it) {
			m_Materials.PushBack((*it)->Clone());
		}
	}
}

Mesh::~Mesh()
{
}

void Mesh::Render(const SceneRenderData& r)
{
	auto node = GetNode();
	if(!node)
		return;

	video::EMaterialTechnique technique;
	ERenderPass toRenderPass;
	switch(r.pass) {
	case ERenderPass::Solid:
	case ERenderPass::Transparent:
		technique = video::EMaterialTechnique::Default;
		toRenderPass = r.pass;
		break;
	case ERenderPass::ShadowCaster:
		technique = video::EMaterialTechnique::ShadowCaster;
		toRenderPass = ERenderPass::Solid;
		break;
	default:
		return;
	}

	const auto worldMat = node->GetAbsoluteTransform().ToMatrix();
	r.video->SetTransform(video::ETransform::World, worldMat);

	video::Geometry* geo = m_Mesh->GetGeometry();
	for(int i = 0; i < m_Mesh->GetRangeCount(); ++i) {
		int matId, firstPrimitive, lastPrimitive;
		m_Mesh->GetMaterialRange(i, matId, firstPrimitive, lastPrimitive);
		video::Material* material = m_OnlyReadMaterials ?
			m_Mesh->GetMaterial(matId) :
			(video::Material*)m_Materials[matId];

		auto pass = GetPassFromReq(material->GetRequirements());

		// Draw transparent geo meshes in transparent pass, and solid in solid path
		if(pass == toRenderPass && firstPrimitive <= lastPrimitive) {
			auto realTechOpt = material->GetTechnique(technique);
			if(realTechOpt.HasValue()) {
				auto realTech = realTechOpt.GetValue();

				video::Material::SetData setData(material);
				r.video->SendPassSettingsEx(
					video::ERenderMode::Mode3D,
					realTech->GetPass(),
					true,
					realTech,
					&setData);
				r.video->Draw(video::RenderRequest::FromGeometry(
					geo,
					firstPrimitive,
					lastPrimitive - firstPrimitive + 1));
			}
		}
	}
}

RenderPassSet Mesh::GetRenderPass() const
{
	RenderPassSet passSet;
	for(int i = 0; i < GetMaterialCount(); ++i) {
		auto mat = GetMaterial(i);
		ERenderPass nextPass = GetPassFromReq(mat->GetRequirements());
		passSet = passSet.Join(nextPass);
	}

	return passSet;
}

const math::AABBoxF& Mesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

video::Material* Mesh::GetMaterial(int index)
{
	if(m_OnlyReadMaterials)
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

const video::Material* Mesh::GetMaterial(int index) const
{
	if(m_OnlyReadMaterials)
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

void Mesh::SetMaterial(int index, video::Material* m)
{
	if(m_OnlyReadMaterials)
		m_Mesh->SetMaterial(index, m);
	else
		m_Materials.At(index) = m;
}

int Mesh::GetMaterialCount() const
{
	if(m_OnlyReadMaterials)
		return m_Mesh->GetMaterialCount();
	else
		return m_Materials.Size();
}

StrongRef<video::Mesh> Mesh::GetMesh()
{
	return m_Mesh;
}

void Mesh::SetMesh(video::Mesh* mesh)
{
	LX_CHECK_NULL_ARG(mesh);
	m_Mesh = mesh;
	m_BoundingBox = mesh->GetBoundingBox();

	CopyMaterials();
}

void Mesh::SetReadMaterialsOnly(bool state)
{
	m_OnlyReadMaterials = state;
	if(!m_OnlyReadMaterials)
		CopyMaterials();
}

bool Mesh::GetReadMaterialsOnly() const
{
	return m_OnlyReadMaterials;
}

void Mesh::CopyMaterials()
{
	m_Materials.Clear();
	if(m_OnlyReadMaterials)
		return;

	int materialCount = m_Mesh->GetMaterialCount();
	m_Materials.Resize(materialCount);
	for(int i = 0; i < materialCount; ++i)
		m_Materials[i] = m_Mesh->GetMaterial(i)->Clone();
}

} // namespace scene
} // namespace lux
