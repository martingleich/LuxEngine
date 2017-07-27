#include "QuadRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"

#include "video/MaterialLibrary.h"

#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"

#include "video/VideoDriver.h"

namespace lux
{
namespace scene
{

QuadRendererMachine::QuadRendererMachine() :
	m_Driver(video::VideoDriver::Instance()),
	m_Buffer(nullptr)
{
	m_BaseMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleTransparent");
	m_EmitMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleEmit");
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

void QuadRendererMachine::Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer)
{
	if(group->GetParticleCount() == 0)
		return;

	CreateBuffers(group);

	StrongRef<video::VertexBuffer> vertexBuffer = m_Buffer->GetVertices();
	const core::Pool<Particle>& pool = group->GetPool();

	m_Data = dynamic_cast<QuadRenderer*>(renderer);
	if(!m_Data)
		return;

	m_Model = group->GetModel();

	auto& mat = m_Data->EmitLight ? m_EmitMaterial : m_BaseMaterial;

	void (QuadRendererMachine::*RenderQuad)(video::Vertex3D* vertices, const Particle& particle);

	{
		video::Texture* texture;
		math::RectF* rect;
		int offset = m_Model->GetOffset(Particle::EParameter::Sprite);
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
			mat->Layer(0) = texture;

		if(m_Model->IsEnabled(Particle::EParameter::Angle))
			RenderQuad = &QuadRendererMachine::RenderQuad_ScaledRotated;
		else
			RenderQuad = &QuadRendererMachine::RenderQuad_Scaled;
	}

	math::Matrix4 world = videoRenderer->GetTransform(video::ETransform::World);
	math::Matrix4 view = videoRenderer->GetTransform(video::ETransform::View);
	math::Matrix4 invWorldView(world * view, math::Matrix4::M4C_INV);

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

	videoRenderer->SetMaterial(mat);
	videoRenderer->DrawGeometry(m_Buffer, pool.GetActiveCount() * 2);
}

void QuadRendererMachine::RenderQuad_Scaled(video::Vertex3D* vertices, const Particle& particle)
{
	float Size = m_Model->ReadValue(particle, Particle::EParameter::Size);

	math::Vector3F Side = m_Side * m_Data->Scaling.x * Size;
	math::Vector3F Up = m_Up * m_Data->Scaling.y * Size;
	if(m_Data->ScaleLengthSpeedSq)
		Up *= particle.velocity.GetLengthSq()*m_Data->ScaleLengthSpeedSq;

	vertices[0].position = particle.position + (Up - Side)*Size;
	vertices[1].position = particle.position + (Up + Side)*Size;
	vertices[2].position = particle.position - (Up + Side)*Size;
	vertices[3].position = particle.position - (Up - Side)*Size;

	float alpha = m_Model->ReadValue(particle, Particle::EParameter::Alpha);
	float red = m_Model->ReadValue(particle, Particle::EParameter::Red);
	float green = m_Model->ReadValue(particle, Particle::EParameter::Green);
	float blue = m_Model->ReadValue(particle, Particle::EParameter::Blue);
	vertices[0].color.Set(alpha, red, green, blue);
	vertices[1].color = vertices[2].color = vertices[3].color = vertices[0].color;

	int offset = m_Model->GetOffset(Particle::EParameter::Sprite);
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
	if(m_Data->SpriteBank->GetSprite(sprite, (u32)(1000.0f * particle.age), true, rect, texture)) {
		vertices[0].texture.Set(rect->left, rect->top);
		vertices[1].texture.Set(rect->right, rect->top);
		vertices[2].texture.Set(rect->left, rect->bottom);
		vertices[3].texture.Set(rect->right, rect->bottom);
	}
}

void QuadRendererMachine::RenderQuad_ScaledRotated(video::Vertex3D* vertices, const Particle& particle)
{
	float Size = m_Model->ReadValue(particle, Particle::EParameter::Size);
	float angle = m_Model->ReadValue(particle, Particle::EParameter::Angle);

	float sa = sinf(angle);
	float ca = cosf(angle);

	math::Vector3F Side = (ca*m_Side - sa*m_Up)* m_Data->Scaling.x * Size;
	math::Vector3F Up = (sa*m_Side + ca*m_Up) * m_Data->Scaling.y * Size;
	if(m_Data->ScaleLengthSpeedSq)
		Up *= particle.velocity.GetLengthSq()*m_Data->ScaleLengthSpeedSq;

	vertices[0].position = particle.position + (Up - Side)*Size;
	vertices[1].position = particle.position + (Up + Side)*Size;
	vertices[2].position = particle.position - (Up + Side)*Size;
	vertices[3].position = particle.position - (Up - Side)*Size;

	float alpha = m_Model->ReadValue(particle, Particle::EParameter::Alpha);
	float red = m_Model->ReadValue(particle, Particle::EParameter::Red);
	float green = m_Model->ReadValue(particle, Particle::EParameter::Green);
	float blue = m_Model->ReadValue(particle, Particle::EParameter::Blue);
	vertices[0].color.Set(alpha, red, green, blue);
	vertices[1].color = vertices[2].color = vertices[3].color = vertices[0].color;

	int offset = m_Model->GetOffset(Particle::EParameter::Sprite);
	video::SpriteBank::Sprite sprite;
	if(offset < 0)
		sprite = m_Data->DefaultSprite;
	else {
		const int spriteID = (int)particle.Param(offset);
		sprite = video::SpriteBank::Sprite(spriteID);
	}

	math::RectF* rect;
	video::Texture* texture;
	if(m_Data->SpriteBank->GetSprite(sprite, (u32)(1000.0f * particle.age), true, rect, texture)) {
		vertices[0].texture.Set(rect->left, rect->top);
		vertices[1].texture.Set(rect->right, rect->top);
		vertices[2].texture.Set(rect->left, rect->bottom);
		vertices[3].texture.Set(rect->right, rect->bottom);
	}
}

void QuadRendererMachine::SetIndexBuffer(video::IndexBuffer* indexBuffer, u32 From, u32 To)
{
	u16 index = (u16)(From / 6) * 4;
	indexBuffer->SetCursor(From);
	for(u32 i = From; i < To;) {
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
	u32 maxParticleCount = group->GetPool().Capactity();
	if(m_Buffer == nullptr) {
		m_Buffer = m_Driver->CreateGeometry(
			video::VertexFormat::STANDARD, video::EHardwareBufferMapping::Dynamic, 4 * maxParticleCount,
			video::EIndexFormat::Bit16, video::EHardwareBufferMapping::Static, 6 * maxParticleCount,
			video::EPrimitiveType::Triangles);

		SetIndexBuffer(m_Buffer->GetIndices(), 0, 6 * group->GetPool().Capactity());
	}

	if(m_Buffer->GetVertices()->GetAlloc() < maxParticleCount * 4) {
		u32 From = m_Buffer->GetIndices()->GetSize();
		m_Buffer->GetVertices()->SetSize(maxParticleCount * 4, false);
		m_Buffer->GetIndices()->SetSize(maxParticleCount * 6);
		SetIndexBuffer(m_Buffer->GetIndices(), From, maxParticleCount * 6);
	}
}

core::Name QuadRendererMachine::GetType() const
{
	static const core::Name name = "quad";
	return name;
}

StrongRef<ParticleRenderer> QuadRendererMachine::CreateRenderer()
{
	return LUX_NEW(QuadRenderer(this));
}

} // namespace scene
} // namespace lux