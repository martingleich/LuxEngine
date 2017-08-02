#include "scene/components/SceneMesh.h"
#include "scene/Node.h"

#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/Renderer.h"
#include "video/MaterialRenderer.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.comp.Mesh", lux::scene::Mesh)

namespace lux
{
namespace scene
{

Mesh::Mesh() :
	m_DirtyMaterials(true),
	m_OnlyReadMaterials(true)
{
}

Mesh::Mesh(const Mesh& other) :
	m_Mesh(other.m_Mesh),
	m_DirtyMaterials(true),
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

void Mesh::VisitRenderables(RenderableVisitor* visitor, bool noDebug)
{
	LUX_UNUSED(noDebug);

	if(m_Mesh)
		visitor->Visit(this);
}

void Mesh::Render(Node* node, video::Renderer* renderer, const SceneData& sceneData)
{
	const auto worldMat = node->GetAbsoluteTransform().ToMatrix();
	renderer->SetTransform(video::ETransform::World, worldMat);

	const bool isTransparentPass = (sceneData.pass == ERenderPass::Transparent);
	for(size_t i = 0; i < m_Mesh->GetSubMeshCount(); ++i) {
		video::Geometry* geo = m_Mesh->GetGeometry(i);
		const video::Material* material = m_OnlyReadMaterials ? m_Mesh->GetMaterial(i) : (const video::Material*)m_Materials[i];

		video::MaterialRenderer* matRenderer = nullptr;
		if(material)
			matRenderer = material->GetRenderer();

		// Default to solid if not more information
		bool isTransparent = false;
		if(matRenderer)
			isTransparent = TestFlag(matRenderer->GetRequirements(), video::MaterialRenderer::ERequirement::Transparent);

		// Draw transparent geo meshes in transparent pass, and solid in solid path
		if(isTransparent == isTransparentPass) {
			renderer->SetMaterial(material);
			renderer->DrawGeometry(geo);
		}
	}
}

ERenderPass Mesh::GetRenderPass() const
{
	if(!m_DirtyMaterials)
		return m_RenderPass;

	m_DirtyMaterials = false;

	bool transparent = false;
	bool solid = false;
	for(size_t i = 0; i < GetMaterialCount(); ++i) {
		const video::Material* Mat = GetMaterial(i);

		// Determine the type of renderer used
		video::MaterialRenderer* renderer = Mat ? Mat->GetRenderer() : nullptr;
		if(renderer && TestFlag(renderer->GetRequirements(), video::MaterialRenderer::ERequirement::Transparent))
			transparent = true;
		else
			solid = true;

		// More than both can never be set
		if(solid && transparent)
			break;
	}

	if(solid && transparent)
		m_RenderPass = ERenderPass::SolidAndTransparent;
	else if(solid)
		m_RenderPass = ERenderPass::Solid;
	else if(transparent)
		m_RenderPass = ERenderPass::Transparent;
	else
		m_RenderPass = ERenderPass::None;

	return m_RenderPass;
}

const math::AABBoxF& Mesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

video::Material* Mesh::GetMaterial(size_t index)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetSubMeshCount())
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

const video::Material* Mesh::GetMaterial(size_t index) const
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetSubMeshCount())
		return m_Mesh->GetMaterial(index);
	else
		return m_Materials.At(index);
}

void Mesh::SetMaterial(size_t index, video::Material* m)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetSubMeshCount())
		m_Mesh->SetMaterial(index, m);
	else
		m_Materials.At(index) = m;

	m_DirtyMaterials = true;
}

size_t Mesh::GetMaterialCount() const
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr)
		return m_Mesh->GetSubMeshCount();
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
	m_DirtyMaterials = true;

	m_Materials.Clear();
	if(m_OnlyReadMaterials || !m_Mesh)
		return;

	size_t materialCount = m_Mesh->GetSubMeshCount();
	m_Materials.Reserve(materialCount);
	for(size_t i = 0; i < materialCount; ++i) {
		auto material = m_Mesh->GetMaterial(i)->Clone();
		m_Materials.PushBack(material);
	}
}

core::Name Mesh::GetReferableType() const
{
	return SceneComponentType::Mesh;
}

StrongRef<Referable> Mesh::Clone() const
{
	return LUX_NEW(Mesh(*this));
}

} // namespace scene
} // namespace lux
