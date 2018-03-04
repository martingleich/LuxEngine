#include "scene/components/SceneMesh.h"
#include "scene/Node.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::Mesh, "lux.comp.Mesh");

namespace lux
{
namespace scene
{

static ERenderPass GetPassFromReq(video::EMaterialRequirement req)
{
	if(TestFlag(req, video::EMaterialRequirement::DeferredEffect))
		return ERenderPass::DeferredEffect;
	else if(TestFlag(req, video::EMaterialRequirement::Transparent))
		return ERenderPass::Transparent;
	else
		return ERenderPass::Solid;
}

Mesh::Mesh() :
	m_OnlyReadMaterials(true)
{
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

void Mesh::VisitRenderables(RenderableVisitor* visitor, ERenderableTags tags)
{
	LUX_UNUSED(tags);

	if(m_Mesh)
		visitor->Visit(GetParent(), this);
}

void Mesh::Render(Node* node, video::Renderer* renderer, const SceneData& sceneData)
{
	const auto worldMat = node->GetAbsoluteTransform().ToMatrix();
	renderer->SetTransform(video::ETransform::World, worldMat);

	video::Geometry* geo = m_Mesh->GetGeometry();
	for(size_t i = 0; i < m_Mesh->GetRangeCount(); ++i) {
		size_t matId, firstPrimitive, lastPrimitive;
		m_Mesh->GetMaterialRange(i, matId, firstPrimitive, lastPrimitive);
		video::Material* material = m_OnlyReadMaterials ?
			m_Mesh->GetMaterial(matId) :
			(video::Material*)m_Materials[matId];

		auto pass = GetPassFromReq(material->GetRequirements());

		// Draw transparent geo meshes in transparent pass, and solid in solid path
		if(pass == sceneData.pass && firstPrimitive < lastPrimitive) {
			renderer->SetMaterial(material);
			renderer->DrawGeometry(
				geo,
				firstPrimitive,
				lastPrimitive - firstPrimitive + 1);
		}
	}
}

ERenderPass Mesh::GetRenderPass() const
{
	ERenderPass pass = ERenderPass::None;
	for(size_t i = 0; i < GetMaterialCount(); ++i) {
		auto mat = GetMaterial(i);
		ERenderPass nextPass = GetPassFromReq(mat->GetRequirements());

		if(pass != ERenderPass::None && pass != nextPass)
			return ERenderPass::Any;
		pass = nextPass;
	}

	m_RenderPass = pass;
	return m_RenderPass;
}

const math::AABBoxF& Mesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

video::Material* Mesh::GetMaterial(size_t index)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetMaterialCount())
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

const video::Material* Mesh::GetMaterial(size_t index) const
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetMaterialCount())
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

void Mesh::SetMaterial(size_t index, video::Material* m)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetMaterialCount())
		m_Mesh->SetMaterial(index, m);
	else
		m_Materials.At(index) = m;
}

size_t Mesh::GetMaterialCount() const
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr)
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
	m_Mesh = mesh;
	if(m_Mesh)
		m_BoundingBox = mesh->GetBoundingBox();
	else
		m_BoundingBox = math::AABBoxF::EMPTY;

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
	if(m_OnlyReadMaterials || !m_Mesh)
		return;

	size_t materialCount = m_Mesh->GetMaterialCount();
	m_Materials.Reserve(materialCount);
	for(size_t i = 0; i < materialCount; ++i) {
		auto material = m_Mesh->GetMaterial(i)->Clone();
		m_Materials.PushBack(material);
	}
}

} // namespace scene
} // namespace lux
