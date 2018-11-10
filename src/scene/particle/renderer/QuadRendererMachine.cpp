#include "scene/particle/renderer/QuadRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"

#include "video/MaterialLibrary.h"

#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "video/VideoDriver.h"

#include "video/Texture.h"

namespace lux
{
namespace scene
{
namespace
{
class ShaderParamLoader : public video::ShaderParamSetCallback
{
public:
	int m_TexId;
	void Init(video::Shader* shader)
	{
		m_TexId = shader->GetParamId("texture");
	}

	void SendShaderSettings(const video::Pass& pass, void* texLayer) const
	{
		pass.shader->SetParam(m_TexId, texLayer);
	}
};

ShaderParamLoader g_ParamLoader;
}

static WeakRef<QuadRendererMachine> g_SharedInstance;
StrongRef<QuadRendererMachine> QuadRendererMachine::GetShared()
{
	if(g_SharedInstance)
		return g_SharedInstance.GetStrong();
	StrongRef<QuadRendererMachine> out = LUX_NEW(QuadRendererMachine);
	g_SharedInstance = out.GetWeak();;
	return out;
}

QuadRendererMachine::QuadRendererMachine() :
	m_Driver(video::VideoDriver::Instance()),
	m_Buffer(nullptr)
{
	m_DefaultPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_DefaultPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_DefaultPass.alpha.blendOperator = video::EBlendOperator::Add;
	m_DefaultPass.zWriteEnabled = false;
	m_DefaultPass.fogEnabled = false;
	m_DefaultPass.lighting = video::ELightingFlag::Disabled;
	video::TextureStageSettings tss;
	tss.alphaArg1 = video::ETextureArgument::Texture;
	tss.alphaArg2 = video::ETextureArgument::Diffuse;
	tss.alphaOperator = video::ETextureOperator::Modulate;
	tss.colorArg1 = video::ETextureArgument::Texture;
	tss.colorArg2 = video::ETextureArgument::Diffuse;
	tss.colorOperator = video::ETextureOperator::Modulate;
	video::FixedFunctionParameters paramDefault({"texture"}, {tss}, true);
	m_DefaultPass.shader = video::MaterialLibrary::Instance()->GetFixedFunctionShader(paramDefault);

	m_EmitPass = m_DefaultPass;
	m_EmitPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_EmitPass.alpha.dstFactor = video::EBlendFactor::One;
	m_EmitPass.alpha.blendOperator = video::EBlendOperator::Add;

	g_ParamLoader.Init(m_DefaultPass.shader);
}

QuadRendererMachine::~QuadRendererMachine()
{
}

bool QuadRendererMachine::PrecomputeOrientation(const math::Matrix4& invModelView)
{
	bool globalOrientation = true;
	switch(m_Data->LookOrient) {
	case ELookOrientation::CameraPlane:
		m_HelpLook.x = invModelView(2, 0);
		m_HelpLook.y = invModelView(2, 1);
		m_HelpLook.z = invModelView(2, 2);
		break;

	case ELookOrientation::CameraPoint:
		m_HelpLook.x = invModelView(3, 0);
		m_HelpLook.y = invModelView(3, 1);
		m_HelpLook.z = invModelView(3, 2);
		globalOrientation = false;
		break;

	case ELookOrientation::Point:
		m_HelpLook = m_Data->LookValue;
		globalOrientation = false;
		break;

	case ELookOrientation::Axis:
		m_HelpLook = m_Data->LookValue;
		break;
	}

	switch(m_Data->UpOrient) {
	case EUpOrientation::Camera:
		m_HelpUp.x = invModelView(1, 0);
		m_HelpUp.y = invModelView(1, 1);
		m_HelpUp.z = invModelView(1, 2);
		break;

	case EUpOrientation::Direction:
		globalOrientation = false;
		break;

	case EUpOrientation::Point:
		m_HelpUp = m_Data->UpValue;
		globalOrientation = false;
		break;

	case EUpOrientation::Axis:
		m_HelpUp = m_Data->UpValue;
		break;
	}

	return globalOrientation;
}

void QuadRendererMachine::ComputeGlobalOrientation()
{
	m_Look = m_HelpLook;
	m_Up = m_HelpUp;
	if(m_Look == m_Up) {
		if(m_Data->LockedAxis == ELockedAxis::Look)
			m_Up = m_Look.GetOrthoNormal();
		else
			m_Look = m_Up.GetOrthoNormal();
	}

	m_Side = m_Look.Cross(m_Up);
	if(m_Data->LockedAxis == ELockedAxis::Look) {
		m_Up = m_Look.Cross(m_Side);
	} else if(m_IsRotationEnabled) {
		m_Look = m_Side.Cross(m_Up);
		m_Look.Normalize();
	}

	m_Up.SetLength(0.5f);
	m_Side.SetLength(0.5f);
}

void QuadRendererMachine::ComputeLocalOrientation(const Particle& particle)
{
	m_Look = m_HelpLook;
	if(m_Data->LookOrient == ELookOrientation::CameraPoint ||
		m_Data->LookOrient == ELookOrientation::Point) {
		m_Look -= particle.position;
	}

	if(m_Data->UpOrient == EUpOrientation::Direction) {
		m_Up = particle.velocity;
	} else if(m_Data->UpOrient == EUpOrientation::Point) {
		m_Up = m_HelpUp;
		m_Up -= particle.position;
	} else {
		m_Up = m_HelpUp;
	}

	if(m_Look == m_Up) {
		if(m_Data->LockedAxis == ELockedAxis::Look)
			m_Up = m_Look.GetOrthoNormal();
		else
			m_Look = m_Up.GetOrthoNormal();
	}
	m_Side = m_Look.Cross(m_Up);
	if(m_Data->LockedAxis == ELockedAxis::Look) {
		m_Up = m_Look.Cross(m_Side);
	} else if(m_IsRotationEnabled) {
		m_Look = m_Side.Cross(m_Up);
		m_Look.Normalize();
	}

	m_Up.SetLength(0.5f);
	m_Side.SetLength(0.5f);
}

void QuadRendererMachine::Render(video::Renderer* videoRenderer, ParticleGroupData* group, QuadRenderer* renderer)
{
	if(group->GetParticleCount() == 0)
		return;

	CreateBuffers(group);

	StrongRef<video::VertexBuffer> vertexBuffer = m_Buffer->GetVertices();
	const core::Pool<Particle>& pool = group->GetPool();

	m_Data = renderer;
	if(!m_Data)
		return;

	m_Model = group->GetModel();

	auto& pass = m_Data->EmitLight ? m_EmitPass : m_DefaultPass;

	void (QuadRendererMachine::*RenderQuad)(video::Vertex3D* vertices, const Particle& particle);

	video::TextureLayer particleTexture;
	{
		video::Texture* texture;
		math::RectF* rect;
		int offset = m_Model->GetParamOffset(ParticleParam::Sprite);
		video::SpriteBank::Sprite sprite;
		if(offset < 0)
			sprite = m_Data->DefaultSprite;
		else {
			const Particle& particle = *group->GetPool().First();
			const int spriteID = (int)particle.Param(offset);
			sprite = video::SpriteBank::Sprite(spriteID);
		}
		// TODO: Allow more than one texture for particle system.
		if(m_Data->SpriteBank->GetSprite(sprite, 0, false, rect, texture))
			particleTexture = video::TextureLayer(texture);

		if(m_Model->IsEnabled(ParticleParam::Angle))
			RenderQuad = &QuadRendererMachine::RenderQuad_ScaledRotated;
		else
			RenderQuad = &QuadRendererMachine::RenderQuad_Scaled;
	}

	math::Matrix4 world = videoRenderer->GetTransform(video::ETransform::World);
	math::Matrix4 view = videoRenderer->GetTransform(video::ETransform::View);
	math::Matrix4 invWorldView = (view * world).GetTransformInverted();

	bool globalOrientation = PrecomputeOrientation(invWorldView);
	if(globalOrientation)
		ComputeGlobalOrientation();

	vertexBuffer->SetCursor(0);
	int i = 0;
	for(core::Pool<Particle>::ConstIterator it = pool.First(); it != pool.End(); ++it) {
		if(globalOrientation == false)
			ComputeLocalOrientation(*it);

		video::Vertex3D Vertices[4];
		Vertices[0].normal = Vertices[1].normal = Vertices[2].normal = Vertices[3].normal = -m_Look;

		(this->*RenderQuad)(Vertices, *it);

		vertexBuffer->AddVertices(Vertices, 4);
		i++;
	}
	vertexBuffer->Update();

	videoRenderer->SetPass(pass, true, &g_ParamLoader, &particleTexture);
	videoRenderer->DrawGeometry(m_Buffer, 0, (int)(pool.GetActiveCount() * 2));
}

void QuadRendererMachine::RenderQuad_Scaled(video::Vertex3D* vertices, const Particle& particle)
{
	float Size = m_Model->ReadValue(particle, ParticleParam::Size);

	math::Vector3F Side = m_Side * m_Data->Scaling.x * Size;
	math::Vector3F Up = m_Up * m_Data->Scaling.y * Size;
	if(m_Data->ScaleLengthSpeedSq)
		Up *= particle.velocity.GetLengthSq()*m_Data->ScaleLengthSpeedSq;

	vertices[0].position = particle.position + (Up - Side)*Size;
	vertices[1].position = particle.position + (Up + Side)*Size;
	vertices[2].position = particle.position - (Up + Side)*Size;
	vertices[3].position = particle.position - (Up - Side)*Size;

	float alpha = m_Model->ReadValue(particle, ParticleParam::Alpha);
	float red = m_Model->ReadValue(particle, ParticleParam::Red);
	float green = m_Model->ReadValue(particle, ParticleParam::Green);
	float blue = m_Model->ReadValue(particle, ParticleParam::Blue);
	vertices[0].color.SetF(alpha, red, green, blue);
	vertices[1].color = vertices[2].color = vertices[3].color = vertices[0].color;

	int offset = m_Model->GetParamOffset(ParticleParam::Sprite);
	video::SpriteBank::Sprite sprite;
	if(offset < 0)
		sprite = m_Data->DefaultSprite;
	else {
		const int spriteID = (int)particle.Param(offset);
		sprite = video::SpriteBank::Sprite(spriteID);
	}

	math::RectF* rect;
	video::Texture* texture;
	// Spritebank works in milliseconds instead of seconds
	if(m_Data->SpriteBank->GetSprite(sprite, (int)(1000.0f * particle.age), true, rect, texture)) {
		vertices[0].texture.Set(rect->left, rect->top);
		vertices[1].texture.Set(rect->right, rect->top);
		vertices[2].texture.Set(rect->left, rect->bottom);
		vertices[3].texture.Set(rect->right, rect->bottom);
	}
}

void QuadRendererMachine::RenderQuad_ScaledRotated(video::Vertex3D* vertices, const Particle& particle)
{
	float Size = m_Model->ReadValue(particle, ParticleParam::Size);
	float angle = m_Model->ReadValue(particle, ParticleParam::Angle);

	float sa = std::sin(angle);
	float ca = std::cos(angle);

	math::Vector3F Side = (ca*m_Side - sa*m_Up)* m_Data->Scaling.x * Size;
	math::Vector3F Up = (sa*m_Side + ca*m_Up) * m_Data->Scaling.y * Size;
	if(m_Data->ScaleLengthSpeedSq)
		Up *= particle.velocity.GetLengthSq()*m_Data->ScaleLengthSpeedSq;

	vertices[0].position = particle.position + (Up - Side)*Size;
	vertices[1].position = particle.position + (Up + Side)*Size;
	vertices[2].position = particle.position - (Up + Side)*Size;
	vertices[3].position = particle.position - (Up - Side)*Size;

	float alpha = m_Model->ReadValue(particle, ParticleParam::Alpha);
	float red = m_Model->ReadValue(particle, ParticleParam::Red);
	float green = m_Model->ReadValue(particle, ParticleParam::Green);
	float blue = m_Model->ReadValue(particle, ParticleParam::Blue);
	vertices[0].color.SetF(alpha, red, green, blue);
	vertices[1].color = vertices[2].color = vertices[3].color = vertices[0].color;

	int offset = m_Model->GetParamOffset(ParticleParam::Sprite);
	video::SpriteBank::Sprite sprite;
	if(offset < 0)
		sprite = m_Data->DefaultSprite;
	else {
		const int spriteID = (int)particle.Param(offset);
		sprite = video::SpriteBank::Sprite(spriteID);
	}

	math::RectF* rect;
	video::Texture* texture;
	if(m_Data->SpriteBank->GetSprite(sprite, (int)(1000.0f * particle.age), true, rect, texture)) {
		vertices[0].texture.Set(rect->left, rect->top);
		vertices[1].texture.Set(rect->right, rect->top);
		vertices[2].texture.Set(rect->left, rect->bottom);
		vertices[3].texture.Set(rect->right, rect->bottom);
	}
}

void QuadRendererMachine::SetIndexBuffer(video::IndexBuffer* indexBuffer, int From, int To)
{
	u16 index = (u16)(From / 6) * 4;
	indexBuffer->SetCursor(From);
	for(int i = From; i < To;) {
		u16 indices[6] = {
			(u16)(index + 0), (u16)(index + 1), (u16)(index + 2),
			(u16)(index + 3), (u16)(index + 2), (u16)(index + 1)};
		indexBuffer->SetIndices(indices, 6, i);

		i += 6;
		index += 4;
	}

	indexBuffer->Update();
}

void QuadRendererMachine::CreateBuffers(ParticleGroupData* group)
{
	int maxParticleCount = group->GetPool().Capactity();
	if(m_Buffer == nullptr) {
		m_Buffer = m_Driver->CreateGeometry(
			video::VertexFormat::STANDARD, video::EHardwareBufferMapping::Dynamic, 4 * maxParticleCount,
			video::EIndexFormat::Bit16, video::EHardwareBufferMapping::Static, 6 * maxParticleCount,
			video::EPrimitiveType::Triangles);

		SetIndexBuffer(m_Buffer->GetIndices(), 0, 6 * group->GetPool().Capactity());
	}

	if(m_Buffer->GetVertices()->GetAlloc() < maxParticleCount * 4) {
		int From = m_Buffer->GetIndices()->GetSize();
		m_Buffer->GetVertices()->SetSize(maxParticleCount * 4, false);
		m_Buffer->GetIndices()->SetSize(maxParticleCount * 6);
		SetIndexBuffer(m_Buffer->GetIndices(), From, maxParticleCount * 6);
	}
}

} // namespace scene
} // namespace lux