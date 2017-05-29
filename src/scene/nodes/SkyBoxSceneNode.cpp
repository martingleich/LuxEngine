#include "scene/nodes/SkyBoxSceneNode.h"

#include "scene/SceneManager.h"
#include "scene/nodes/CameraSceneNode.h"

#include "video/MaterialLibrary.h"
#include "video/Renderer.h"
#include "video/VertexFormats.h"
#include "video/PipelineSettings.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::SkyBoxSceneNode)

namespace lux
{
namespace scene
{

void SkyBoxSceneNode::OnRegisterSceneNode()
{
	GetSceneManager()->RegisterNodeForRendering(this, ESNRP_SKY_BOX);

	SceneNode::OnRegisterSceneNode();
}

bool SkyBoxSceneNode::SetSceneManager(SceneManager* mgr)
{
	m_Material = mgr->GetMaterialLibrary()->CreateMaterial();
	return SceneNode::SetSceneManager(mgr);
}

void SkyBoxSceneNode::Render()
{
	video::Renderer* renderer = GetSceneManager()->GetRenderer();
	const CameraSceneNode* camera = GetSceneManager()->GetActiveCamera();

	if(!renderer || !camera)
		return;

	math::Transformation t = this->GetAbsoluteTransform();

	t.translation = camera->GetAbsolutePosition();

	// Die Sky-box genau zwischen die Clipping-Ebenen legen
	t.scale = 0.5f * (camera->GetNearPlane() + camera->GetFarPlane());

	math::matrix4 m;
	t.ToMatrix(m);
	renderer->SetTransform(video::ETransform::World, m);

	// Adjust Pipeline for Skyboxes
	video::PipelineOverwrite over;
	over.disableFog = true;
	over.disableLighting = true;
	over.disableZWrite = true;
	renderer->PushPipelineOverwrite(over);
	
	// Die Vertizes
	static const video::Vertex3DTCoord Vertices[8] =
	{
		video::Vertex3DTCoord(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f),
		video::Vertex3DTCoord(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		video::Vertex3DTCoord(1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f),
		video::Vertex3DTCoord(-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f),
		video::Vertex3DTCoord(-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f),
		video::Vertex3DTCoord(1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f),
		video::Vertex3DTCoord(1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f),
		video::Vertex3DTCoord(-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f)
	};

	static const u16 ausIndex[36] =
	{
		7, 3, 0, 4, 7, 0,    // Front
		5, 1, 2, 6, 5, 2,    // Back
		4, 0, 1, 5, 4, 1,    // Left
		6, 2, 3, 7, 6, 3,    // Right
		2, 1, 0, 3, 2, 0,    // Top
		4, 5, 6, 7, 4, 6     // Down
	};

		
	m_Material->Layer(0) = m_SkyTexture;
	renderer->SetMaterial(m_Material);
	renderer->DrawIndexedPrimitiveList(video::EPrimitiveType::Triangles,
		12,
		Vertices,
		8,
		video::VertexFormat::TEXTURE_3D,
		ausIndex,
		video::EIndexFormat::Bit16,
		true);

	renderer->PopPipelineOverwrite();
}

void SkyBoxSceneNode::SetSkyTexture(video::CubeTexture* pSkyTexture)
{
	m_SkyTexture = pSkyTexture;
}

video::Material* SkyBoxSceneNode::GetMaterial(size_t id)
{
	if(id == 0)
		return m_Material;
	else
		return SceneNode::GetMaterial(id);
}

void SkyBoxSceneNode::SetMaterial(size_t i, video::Material* m)
{
	if(i == 0)
		m_Material = m;
	else
		return SceneNode::SetMaterial(i, m);
}

size_t SkyBoxSceneNode::GetMaterialCount() const
{
	return 1;
}

core::Name SkyBoxSceneNode::GetReferableSubType() const
{
	return SceneNodeType::SkyBox;
}

StrongRef<Referable> SkyBoxSceneNode::Clone() const
{
	StrongRef<SkyBoxSceneNode> out = LUX_NEW(SkyBoxSceneNode)(*this);

	return out;
}

}
}