#include "scene/components/SkyBox.h"

#include "scene/components/Camera.h"
#include "scene/Node.h"

#include "video/MaterialLibrary.h"
#include "video/Renderer.h"
#include "video/VertexFormat.h"
#include "video/VertexTypes.h"

#include "math/Transformation.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::SkyBox, "lux.comp.SkyBox");

namespace lux
{
namespace scene
{

SkyBox::SkyBox() :
	m_UseCubeTexture(true)
{
	// TODO: Use custom shader for the skybox.
	auto mat = video::MaterialLibrary::Instance()->TryGetMaterial("skybox");
	if(mat) {
		m_Material = mat->Clone();
	} else {
		auto solid = video::MaterialLibrary::Instance()->GetMaterial("solid");
		video::Pass pass = solid->GetTechnique()->GetPass();
		pass.fogEnabled = false;
		pass.lighting = video::ELightingFlag::Disabled;
		mat = video::MaterialLibrary::Instance()->CreateMaterial(pass);
		video::MaterialLibrary::Instance()->SetMaterial("skybox", mat);
		m_Material = mat;
	}
}

SkyBox::SkyBox(const SkyBox& other) :
	m_SkyTexture(other.m_SkyTexture),
	m_UseCubeTexture(other.m_UseCubeTexture)
{
	m_Material = other.m_Material->Clone();
}

SkyBox::~SkyBox()
{
}

static const video::Vertex3DTCoord g_Vertices3D[8] =
{
	video::Vertex3DTCoord(-1, +1, +1, -1, +1, +1),
	video::Vertex3DTCoord(+1, +1, +1, +1, +1, +1),
	video::Vertex3DTCoord(+1, +1, -1, +1, +1, -1),
	video::Vertex3DTCoord(-1, +1, -1, -1, +1, -1),
	video::Vertex3DTCoord(-1, -1, +1, -1, -1, +1),
	video::Vertex3DTCoord(+1, -1, +1, +1, -1, +1),
	video::Vertex3DTCoord(+1, -1, -1, +1, -1, -1),
	video::Vertex3DTCoord(-1, -1, -1, -1, -1, -1)
};

static const video::Vertex3D g_Vertices2D[] =
{
	// create front side
	video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 0, +1, 1, 1),
	video::Vertex3D(+1, -1, -1, video::Color::Black, 0, 0, +1, 0, 1),
	video::Vertex3D(+1, +1, -1, video::Color::Black, 0, 0, +1, 0, 0),

	video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 0, +1, 1, 1),
	video::Vertex3D(+1, +1, -1, video::Color::Black, 0, 0, +1, 0, 0),
	video::Vertex3D(-1, +1, -1, video::Color::Black, 0, 0, +1, 1, 0),

	// create left side
	video::Vertex3D(+1, -1, -1, video::Color::Black, -1, 0, 0, 1, 1),
	video::Vertex3D(+1, -1, +1, video::Color::Black, -1, 0, 0, 0, 1),
	video::Vertex3D(+1, +1, +1, video::Color::Black, -1, 0, 0, 0, 0),

	video::Vertex3D(+1, -1, -1, video::Color::Black, -1, 0, 0, 1, 1),
	video::Vertex3D(+1, +1, +1, video::Color::Black, -1, 0, 0, 0, 0),
	video::Vertex3D(+1, +1, -1, video::Color::Black, -1, 0, 0, 1, 0),

	// create back side
	video::Vertex3D(+1, -1, +1, video::Color::Black, 0, 0, -1, 1, 1),
	video::Vertex3D(-1, -1, +1, video::Color::Black, 0, 0, -1, 0, 1),
	video::Vertex3D(-1, +1, +1, video::Color::Black, 0, 0, -1, 0, 0),

	video::Vertex3D(+1, -1, +1, video::Color::Black, 0, 0, -1, 1, 1),
	video::Vertex3D(-1, +1, +1, video::Color::Black, 0, 0, -1, 0, 0),
	video::Vertex3D(+1, +1, +1, video::Color::Black, 0, 0, -1, 1, 0),

	// create right side
	video::Vertex3D(-1, -1, +1, video::Color::Black, 1, 0, 0, 1, 1),
	video::Vertex3D(-1, -1, -1, video::Color::Black, 1, 0, 0, 0, 1),
	video::Vertex3D(-1, +1, -1, video::Color::Black, 1, 0, 0, 0, 0),

	video::Vertex3D(-1, -1, +1, video::Color::Black, 1, 0, 0, 1, 1),
	video::Vertex3D(-1, +1, -1, video::Color::Black, 1, 0, 0, 0, 0),
	video::Vertex3D(-1, +1, +1, video::Color::Black, 1, 0, 0, 1, 0),

	// create top side
	video::Vertex3D(+1, +1, -1, video::Color::Black, 0, -1, 0, 1, 1),
	video::Vertex3D(+1, +1, +1, video::Color::Black, 0, -1, 0, 0, 1),
	video::Vertex3D(-1, +1, +1, video::Color::Black, 0, -1, 0, 0, 0),

	video::Vertex3D(+1, +1, -1, video::Color::Black, 0, -1, 0, 1, 1),
	video::Vertex3D(-1, +1, +1, video::Color::Black, 0, -1, 0, 0, 0),
	video::Vertex3D(-1, +1, -1, video::Color::Black, 0, -1, 0, 1, 0),

	// create bottom side
	video::Vertex3D(+1, -1, +1, video::Color::Black, 0, +1, 0, 0, 0),
	video::Vertex3D(+1, -1, -1, video::Color::Black, 0, +1, 0, 1, 0),
	video::Vertex3D(-1, -1, -1, video::Color::Black, 0, +1, 0, 1, 1),

	video::Vertex3D(+1, -1, +1, video::Color::Black, 0, +1, 0, 0, 0),
	video::Vertex3D(-1, -1, -1, video::Color::Black, 0, +1, 0, 1, 1),
	video::Vertex3D(-1, -1, +1, video::Color::Black, 0, +1, 0, 0, 1),
};

static const u16 g_Indices[36] =
{
	7, 3, 0, 4, 7, 0,    // Front
	5, 1, 2, 6, 5, 2,    // Back
	4, 0, 1, 5, 4, 1,    // Left
	6, 2, 3, 7, 6, 3,    // Right
	2, 1, 0, 3, 2, 0,    // Top
	4, 5, 6, 7, 4, 6     // Down
};

void SkyBox::Render(const SceneRenderData& data)
{
	if(data.pass != ERenderPass::SkyBox)
		return;

	if(!m_Material)
		return;
	auto node = GetNode();
	if(!node)
		return;

	math::Transformation t = node->GetAbsoluteTransform();
	t.translation = data.activeCamera->GetNode()->GetAbsolutePosition();

	// Place the skybox right between the clipping planes
	auto frustum = data.activeCamera->GetFrustum();
	auto near = math::GetDistanceFromPlaneToPoint(frustum.Plane(math::ViewFrustum::EPlane::Near), t.translation);
	auto far = math::GetDistanceFromPlaneToPoint(frustum.Plane(math::ViewFrustum::EPlane::Far), t.translation);
	t.scale = 0.5f * (near + far);

	data.video->SetTransform(video::ETransform::World, t.ToMatrix());

	// Disable z comparison for sky box renderering
	video::PipelineOverwrite over;
	over.OverwriteZWrite(false);
	video::PipelineOverwriteToken token;
	data.video->PushPipelineOverwrite(over, &token);

	if(m_SkyTexture)
		m_Material->SetTexture(0, m_SkyTexture);

	data.video->SendMaterialSettings(m_Material);
	if(m_UseCubeTexture) {
		data.video->Draw(video::RenderRequest::IndexedFromMemory(
			video::EPrimitiveType::Triangles, 12,
			g_Vertices3D, 8, video::VertexFormat::TEXTURE_3D,
			g_Indices, video::EIndexFormat::Bit16));
	} else {
		data.video->Draw(video::RenderRequest::FromMemory(
			video::EPrimitiveType::Triangles, 12,
			g_Vertices2D, 36, video::VertexFormat::STANDARD));
	}
}

ERenderPass SkyBox::GetRenderPass() const
{
	return ERenderPass::SkyBox;
}

void SkyBox::UseCubeTexture(bool cube)
{
	m_UseCubeTexture = cube;
}

bool SkyBox::IsUsingCubeTexture() const
{
	return m_UseCubeTexture;
}

void SkyBox::SetSkyTexture(video::BaseTexture* skyTexture)
{
	m_SkyTexture = skyTexture;
}

StrongRef<video::BaseTexture> SkyBox::GetSkyTexture() const
{
	return m_SkyTexture;
}

video::Material* SkyBox::GetMaterial()
{
	return m_Material;
}

const video::Material* SkyBox::GetMaterial() const
{
	return m_Material;
}

void SkyBox::SetMaterial(video::Material* m)
{
	m_Material = m;
}

} // namespace scene
} // namespace lux