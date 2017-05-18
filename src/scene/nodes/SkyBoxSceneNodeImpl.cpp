#include "SkyBoxSceneNodeImpl.h"
#include "scene/SceneManager.h"
#include "video/MaterialLibrary.h"
#include "video/VideoDriver.h"
#include "scene/nodes/CameraSceneNode.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::SkyBoxSceneNodeImpl)

namespace lux
{
namespace scene
{

void SkyBoxSceneNodeImpl::OnRegisterSceneNode()
{
	GetSceneManager()->RegisterNodeForRendering(this, ESNRP_SKY_BOX);

	SceneNode::OnRegisterSceneNode();
}

bool SkyBoxSceneNodeImpl::SetSceneManager(SceneManager* mgr)
{
	m_Material.SetRenderer(mgr->GetMaterialLibrary()->GetMaterialRenderer("solid"));
	return SceneNode::SetSceneManager(mgr);
}

void SkyBoxSceneNodeImpl::Render()
{
	video::VideoDriver* driver = GetSceneManager()->GetDriver();
	const CameraSceneNode* camera = GetSceneManager()->GetActiveCamera();

	if(!driver || !camera)
		return;

	math::Transformation t = this->GetAbsoluteTransform();

	t.translation = camera->GetAbsolutePosition();

	// Die Sky-box genau zwischen die Clipping-Ebenen legen
	t.scale = 0.5f * (camera->GetNearPlane() + camera->GetFarPlane());

	math::matrix4 m;
	t.ToMatrix(m);
	driver->SetTransform(video::ETS_WORLD, m);

	// Adjust Pipeline for Skyboxes
	video::PipelineOverwrite over;
	over.SetFlags(video::EPF_FOG_ENABLED | video::EPF_LIGHTING | video::EPF_ZWRITE_ENABLED | video::EPF_ZBUFFER);
	over.Override.FogEnabled = false;
	over.Override.Lighting = false;
	over.Override.ZWriteEnabled = false;
	over.Override.ZBufferFunc = video::EZComparisonFunc::Always;
	over.Override.UseMIPMaps = false;
	driver->PushPipelineOverwrite(over);
	
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

	// Die Indizes
	static const u16 ausIndex[36] =
	{
		7, 3, 0, 4, 7, 0,    // Vorderseite
		5, 1, 2, 6, 5, 2,    // Hinterseite
		4, 0, 1, 5, 4, 1,    // Linke Seite
		6, 2, 3, 7, 6, 3,    // Rechte Seite
		2, 1, 0, 3, 2, 0,    // Oberseite
		4, 5, 6, 7, 4, 6};// Unterseite

		
	m_Material.Layer(0) = m_SkyTexture;
	driver->Set3DMaterial(m_Material);
	driver->Draw3DPrimitiveList(video::EPT_TRIANGLES,
		12,
		Vertices,
		8,
		video::VertexFormat::TEXTURE_3D,
		ausIndex,
		video::EIndexFormat::Bit16);

	driver->PopPipelineOverwrite();
}

void SkyBoxSceneNodeImpl::SetSkyTexture(video::CubeTexture* pSkyTexture)
{
	m_SkyTexture = pSkyTexture;
}

video::Material& SkyBoxSceneNodeImpl::GetMaterial(size_t id)
{
	if(id == 0)
		return m_Material;
	else
		return video::IdentityMaterial;
}

size_t SkyBoxSceneNodeImpl::GetMaterialCount() const
{
	return 1;
}

core::Name SkyBoxSceneNodeImpl::GetReferableSubType() const
{
	return SceneNodeType::SkyBox;
}

StrongRef<Referable> SkyBoxSceneNodeImpl::Clone() const
{
	StrongRef<SkyBoxSceneNodeImpl> out = LUX_NEW(SkyBoxSceneNodeImpl)(*this);

	return out;
}

}
}