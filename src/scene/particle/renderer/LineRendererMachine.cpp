#include "LineRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"
#include "video/Material.h"
#include "video/MaterialLibrary.h"

namespace lux
{
namespace scene
{

LineRendererMachine::LineRendererMachine()
{
	video::VertexDeclaration decl;
	decl.AddElement(video::VertexElement::EUsage::Position, video::VertexElement::EType::Float3);
	decl.AddElement(video::VertexElement::EUsage::Diffuse, video::VertexElement::EType::Color);
	m_VertexFormat = video::VertexFormat("particleLine", decl);

	m_BaseMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleTransparent");
	m_EmitMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleEmit");
}

LineRendererMachine::~LineRendererMachine()
{
}

void LineRendererMachine::CreateBuffer(ParticleGroupData* group)
{
	if(m_Vertices.Size() < group->GetParticleCount() * 2)
		m_Vertices.Resize(group->GetParticleCount() * 2);
}

void LineRendererMachine::Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer)
{
	if(group->GetParticleCount() == 0)
		return;

	m_Data = dynamic_cast<LineRenderer*>(renderer);
	if(!m_Data)
		return;

	CreateBuffer(group);

	ParticleModel* model = group->GetModel();
	u32 cursor = 0;
	for(auto& p : group->GetPool()) {
		float alpha = model->ReadValue(p, Particle::EParameter::Alpha);
		float red = model->ReadValue(p, Particle::EParameter::Red);
		float green = model->ReadValue(p, Particle::EParameter::Green);
		float blue = model->ReadValue(p, Particle::EParameter::Blue);

		video::Color color;
		color.SetF(alpha, red, green, blue);

		float lSq = p.velocity.GetLengthSq();
		auto delta = p.velocity;
		if(math::IsZero(lSq))
			delta = m_Data->DefaultDir;
		else if(!m_Data->ScaleSpeed)
			delta /= std::sqrt(lSq);
		delta *= m_Data->Length * model->ReadValue(p, Particle::EParameter::Size);

		m_Vertices[cursor].position = p.position;
		m_Vertices[cursor].color = color;
		m_Vertices[cursor + 1].position = p.position + delta;
		m_Vertices[cursor + 1].color = color;

		cursor += 2;
	}

	if(m_Data->EmitLight)
		videoRenderer->SetMaterial(m_EmitMaterial);
	else
		videoRenderer->SetMaterial(m_BaseMaterial);

	videoRenderer->DrawPrimitiveList(
		video::EPrimitiveType::Lines,
		group->GetParticleCount(),
		m_Vertices.Data_c(),
		group->GetParticleCount() * 2,
		m_VertexFormat,
		true);
}

core::Name LineRendererMachine::GetType() const
{
	static const core::Name name("lux.particlerenderer.line");

	return name;
}

StrongRef<ParticleRenderer> LineRendererMachine::CreateRenderer()
{
	return LUX_NEW(LineRenderer)(this);
}

} // namespace scene
} // namespace lux