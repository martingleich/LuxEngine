#include "MeshSceneNodeImpl.h"
#include "video/Renderer.h"
#include "scene/SceneManager.h"
#include "video/SubMesh.h"
#include "core/ReferableRegister.h"
#include "video/MaterialRenderer.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::MeshSceneNodeImpl)

namespace lux
{
namespace scene
{

MeshSceneNodeImpl::MeshSceneNodeImpl() :
	m_Mesh(nullptr),
	m_OnlyReadMaterials(true)
{
}

MeshSceneNodeImpl::~MeshSceneNodeImpl()
{
}

void MeshSceneNodeImpl::OnRegisterSceneNode()
{
	bool bTransparent = false;
	bool bSolid = false;

	// Typ(en) des Knotens feststellen
	if(m_OnlyReadMaterials && m_Mesh != nullptr) {
		for(u32 dwSub = 0; dwSub < m_Mesh->GetSubMeshCount(); ++dwSub) {
			video::SubMesh* subMesh = m_Mesh->GetSubMesh(dwSub);
			video::Material* Mat = subMesh->GetMaterial();

			video::MaterialRenderer* renderer = Mat ? Mat->GetRenderer() : nullptr;
			if(renderer && TestFlag(renderer->GetRequirements(), video::MaterialRenderer::ERequirement::Transparent))
				bTransparent = true;
			else
				bSolid = true;

			if(bSolid && bTransparent)
				break;
		}
	} else {
		for(u32 dwMat = 0; dwMat < m_Materials.Size(); ++dwMat) {
			video::Material* mat = m_Materials[dwMat];
			video::MaterialRenderer* renderer = mat ? mat->GetRenderer() : nullptr;
			if(renderer && TestFlag(renderer->GetRequirements(), video::MaterialRenderer::ERequirement::Transparent))
				bTransparent = true;
			else
				bSolid = true;

			if(bSolid && bTransparent)
				break;
		}
	}

	// Knoten registieren
	if(bSolid)
		GetSceneManager()->RegisterNodeForRendering(this, ESNRP_SOLID);
	if(bTransparent)
		GetSceneManager()->RegisterNodeForRendering(this, ESNRP_TRANSPARENT);

	// Die Kinder registrieren
	SceneNode::OnRegisterSceneNode();
}

void MeshSceneNodeImpl::Render()
{
	video::Renderer* renderer = GetSceneManager()->GetRenderer();

	if(!m_Mesh || !renderer)
		return;

	// Wir müssen aufpassen in welchem Renderpass wir sind
	bool bIsTransparentPass = (GetSceneManager()->GetActRenderPass() == ESNRP_TRANSPARENT);

	m_RenderTransform.Set(this);

	renderer->SetTransform(video::ETransform::World, m_RenderTransform.world);

	// Alles Zeichnen
	for(u32 dwSub = 0; dwSub < m_Mesh->GetSubMeshCount(); ++dwSub) {
		video::SubMesh* pSub = m_Mesh->GetSubMesh(dwSub);
		if(pSub) {
			const video::Material* Mat = m_OnlyReadMaterials ? pSub->GetMaterial() : (const video::Material*)m_Materials[dwSub];

			// renderer abfragen, wenn es denn gewünschten nicht gibt abbrechen
			video::MaterialRenderer* matRenderer = nullptr;
			if(Mat)
				matRenderer = Mat->GetRenderer();
			bool isTransparent = false;
			if(matRenderer)
				isTransparent = TestFlag(matRenderer->GetRequirements(), video::MaterialRenderer::ERequirement::Transparent);

			if(bIsTransparentPass == isTransparent) {
				renderer->SetMaterial(Mat);
				renderer->DrawSubMesh(pSub);
			}
		}
	}

	// Debugdaten zeichnen
	if(this->GetDebugData(EDD_SUB_BBOX)) {
		/*
		TODO:
		for(u32 dwSub = 0; dwSub < m_Mesh->GetSubMeshCount(); ++dwSub) {
			video::SubMesh* pSub = m_Mesh->GetSubMesh(dwSub);
			if(pSub)
				driver->Draw3DBox(pSub->GetBoundingBox(), video::Color::Orange);
		}
		*/
	}

	// Debugdaten zeichnen
	if(this->GetDebugData(EDD_MAIN_BBOX)) {
		/*
		TODO:
		driver->Draw3DBox(m_Mesh->GetBoundingBox(), video::Color::Yellow);
		*/
	}

}

video::Material* MeshSceneNodeImpl::GetMaterial(size_t index)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetSubMeshCount())
		return m_Mesh->GetSubMesh(index)->GetMaterial();

	// Wenn der index zu groß ist, hat vieleicht der Knoten selbst noch Materialien
	if(index >= m_Materials.Size())
		return SceneNode::GetMaterial(index);

	return m_Materials[index];
}

void MeshSceneNodeImpl::SetMaterial(size_t index, video::Material* m)
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr && index < m_Mesh->GetSubMeshCount())
		m_Mesh->GetSubMesh(index)->SetMaterial(m);

	// Wenn der index zu groß ist, hat vieleicht der Knoten selbst noch Materialien
	if(index >= m_Materials.Size())
		SceneNode::SetMaterial(index, m);

	m_Materials[index] = m;
}

size_t MeshSceneNodeImpl::GetMaterialCount() const
{
	if(m_OnlyReadMaterials && m_Mesh != nullptr)
		return m_Mesh->GetSubMeshCount();

	return m_Materials.Size();
}

void MeshSceneNodeImpl::SetMesh(Mesh* mesh)
{
	// Alte mesh löschen, Neue einsetzen
	// Und die Materialien kopieren
	if(mesh != nullptr) {
		m_BoundingBox = mesh->GetBoundingBox();

		m_Mesh = mesh;
		CopyMaterials();
	} else {
		m_BoundingBox = math::aabbox3df::EMPTY;
	}
}

void MeshSceneNodeImpl::CopyMaterials()
{
	if(m_Mesh != nullptr) {
		if(m_OnlyReadMaterials)
			return;

		size_t MaterialCount = m_Mesh->GetSubMeshCount();
		size_t OldCount = m_Materials.Size();

		if(MaterialCount < OldCount)
			m_Materials.Resize(MaterialCount);

		// Wenn mit der Submesh ein Fehler auftritt wird das Standartmaterial benutzt
		for(size_t i = 0; i < MaterialCount; ++i) {
			video::SubMesh* subMesh = m_Mesh->GetSubMesh(i);
			StrongRef<video::Material> material;
			if(subMesh)
				material = subMesh->GetMaterial()->Clone();

			if(i < OldCount)
				m_Materials[i] = material;
			else
				m_Materials.PushBack(material);
		}
	} else {
		m_Materials.Clear();
	}
}

core::Name MeshSceneNodeImpl::GetReferableSubType() const
{
	return SceneNodeType::Mesh;
}

StrongRef<Referable> MeshSceneNodeImpl::Clone() const
{
	StrongRef<MeshSceneNodeImpl> out = LUX_NEW(MeshSceneNodeImpl)(*this);
	out->SetMesh(m_Mesh);

	return out;
}

}
}

