#include "scene/components/SkyBox.h"

#include "scene/SceneManager.h"
#include "scene/components/Camera.h"
#include "scene/Node.h"

#include "video/MaterialLibrary.h"
#include "video/Renderer.h"
#include "video/VertexFormats.h"
#include "video/VertexTypes.h"

#include "math/Transformation.h"

#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.comp.Skybox", lux::scene::SkyBox)

namespace lux
{
namespace scene
{

SkyBox::SkyBox() :
	m_UseCubeTexture(true)
{
	video::MaterialRenderer* renderer;
	if(!video::MaterialLibrary::Instance()->ExistsMaterialRenderer("skyBox", &renderer)) {
		renderer = video::MaterialLibrary::Instance()->CloneMaterialRenderer("skyBox", "solid");
		auto& pass = renderer->GetPass(0);
		pass.fogEnabled = false;
		pass.lighting = false;
		renderer->AddParam("tex", 0, (u32)video::EOptionId::Layer0);
	}

	m_Material = renderer->CreateMaterial();
}

SkyBox::SkyBox(const SkyBox& other)
{
	if(other.m_Material)
		m_Material = other.m_Material->Clone().As<video::Material>();
}

SkyBox::~SkyBox()
{
}

void SkyBox::VisitRenderables(RenderableVisitor* visitor, bool noDebug)
{
	LUX_UNUSED(noDebug);

	visitor->Visit(this);
}

void SkyBox::Render(Node* node, video::Renderer* renderer, ERenderPass pass)
{
	if(pass != ERenderPass::SkyBox)
		return;

	if(!m_Material)
		return;

	const Node* camNode = node->GetSceneManager()->GetActiveCameraNode();
	const Camera* camera = node->GetSceneManager()->GetActiveCamera();

	math::Transformation t = node->GetAbsoluteTransform();
	t.translation = camNode->GetAbsolutePosition();

	// Die Sky-box genau zwischen die Clipping-Ebenen legen
	t.scale = 0.5f * (camera->GetNearPlane() + camera->GetFarPlane());

	math::matrix4 m;
	t.ToMatrix(m);
	renderer->SetTransform(video::ETransform::World, m);

	static const video::Vertex3DTCoord Vertices3D[8] =
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

	static const video::Vertex3D Vertices2D[] =
	{
		// create front side
		video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 0, 1, 1, 1),
		video::Vertex3D(1, -1, -1, video::Color::Black, 0, 0, 1, 0, 1),
		video::Vertex3D(1, 1, -1, video::Color::Black, 0, 0, 1, 0, 0),

		video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 0, 1, 1, 1),
		video::Vertex3D(1, 1, -1, video::Color::Black, 0, 0, 1, 0, 0),
		video::Vertex3D(-1, 1, -1, video::Color::Black, 0, 0, 1, 1, 0),

		// create left side
		video::Vertex3D(1, -1, -1, video::Color::Black, -1, 0, 0, 1, 1),
		video::Vertex3D(1, -1, 1, video::Color::Black, -1, 0, 0, 0, 1),
		video::Vertex3D(1, 1, 1, video::Color::Black, -1, 0, 0, 0, 0),

		video::Vertex3D(1, -1, -1, video::Color::Black, -1, 0, 0, 1, 1),
		video::Vertex3D(1, 1, 1, video::Color::Black, -1, 0, 0, 0, 0),
		video::Vertex3D(1, 1, -1, video::Color::Black, -1, 0, 0, 1, 0),

		// create back side
		video::Vertex3D(1, -1, 1, video::Color::Black, 0, 0, -1, 1, 1),
		video::Vertex3D(-1, -1, 1, video::Color::Black, 0, 0, -1, 0, 1),
		video::Vertex3D(-1, 1, 1, video::Color::Black, 0, 0, -1, 0, 0),

		video::Vertex3D(1, -1, 1, video::Color::Black, 0, 0, -1, 1, 1),
		video::Vertex3D(-1, 1, 1, video::Color::Black, 0, 0, -1, 0, 0),
		video::Vertex3D(1, 1, 1, video::Color::Black, 0, 0, -1, 1, 0),

		// create right side
		video::Vertex3D(-1, -1, 1, video::Color::Black, 1, 0, 0, 1, 1),
		video::Vertex3D(-1, -1, -1, video::Color::Black, 1, 0, 0, 0, 1),
		video::Vertex3D(-1, 1, -1, video::Color::Black, 1, 0, 0, 0, 0),

		video::Vertex3D(-1, -1, 1, video::Color::Black, 1, 0, 0, 1, 1),
		video::Vertex3D(-1, 1, -1, video::Color::Black, 1, 0, 0, 0, 0),
		video::Vertex3D(-1, 1, 1, video::Color::Black, 1, 0, 0, 1, 0),

		// create top side
		video::Vertex3D(1, 1, -1, video::Color::Black, 0, -1, 0, 1, 1),
		video::Vertex3D(1, 1, 1, video::Color::Black, 0, -1, 0, 0, 1),
		video::Vertex3D(-1, 1, 1, video::Color::Black, 0, -1, 0, 0, 0),

		video::Vertex3D(1, 1, -1, video::Color::Black, 0, -1, 0, 1, 1),
		video::Vertex3D(-1, 1, 1, video::Color::Black, 0, -1, 0, 0, 0),
		video::Vertex3D(-1, 1, -1, video::Color::Black, 0, -1, 0, 1, 0),

		// create bottom side
		video::Vertex3D(1, -1, 1, video::Color::Black, 0, 1, 0, 0, 0),
		video::Vertex3D(1, -1, -1, video::Color::Black, 0, 1, 0, 1, 0),
		video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 1, 0, 1, 1),

		video::Vertex3D(1, -1, 1, video::Color::Black, 0, 1, 0, 0, 0),
		video::Vertex3D(-1, -1, -1, video::Color::Black, 0, 1, 0, 1, 1),
		video::Vertex3D(-1, -1, 1, video::Color::Black, 0, 1, 0, 0, 1),
	};

	static const u16 Indices[36] =
	{
		7, 3, 0, 4, 7, 0,    // Front
		5, 1, 2, 6, 5, 2,    // Back
		4, 0, 1, 5, 4, 1,    // Left
		6, 2, 3, 7, 6, 3,    // Right
		2, 1, 0, 3, 2, 0,    // Top
		4, 5, 6, 7, 4, 6     // Down
	};

	// Disable z comparison for sky box renderering
	video::PipelineOverwrite over;
	over.disableZCmp = true;
	video::PipelineOverwriteToken tok(renderer, over);

	if(m_SkyTexture)
		m_Material->Layer(0) = m_SkyTexture;

	renderer->SetMaterial(m_Material);
	if(m_UseCubeTexture) {
		renderer->DrawIndexedPrimitiveList(
			video::EPrimitiveType::Triangles,
			12,
			Vertices3D,
			8,
			video::VertexFormat::TEXTURE_3D,
			Indices,
			video::EIndexFormat::Bit16,
			true);
	} else {
		renderer->DrawPrimitiveList(
			video::EPrimitiveType::Triangles,
			12,
			Vertices2D,
			36,
			video::VertexFormat::STANDARD,
			true);
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

video::Material* SkyBox::GetMaterial(size_t i)
{
	if(i > 0)
		throw core::OutOfRangeException();
	else
		return m_Material;
}

const video::Material* SkyBox::GetMaterial(size_t i) const
{
	if(i > 0)
		throw core::OutOfRangeException();
	else
		return m_Material;
}

void SkyBox::SetMaterial(size_t i, video::Material* m)
{
	if(i > 0)
		throw core::OutOfRangeException();
	else
		m_Material = m;
}

size_t SkyBox::GetMaterialCount() const
{
	return 1;
}

const math::aabbox3df& SkyBox::GetBoundingBox() const
{
	return math::aabbox3df::EMPTY;
}

core::Name SkyBox::GetReferableType() const
{
	return SceneComponentType::SkyBox;
}

StrongRef<Referable> SkyBox::Clone() const
{
	return LUX_NEW(SkyBox)(*this);
}

} // namespace scene
} // namespace lux