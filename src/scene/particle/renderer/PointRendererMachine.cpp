#include "PointRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"

#include "video/MaterialLibrary.h"

namespace lux
{
namespace scene
{

PointRendererMachine::PointRendererMachine()
{
	video::VertexDeclaration decl;
	decl.AddElement(video::VertexElement::EUsage::Position, video::VertexElement::EType::Float3);
	decl.AddElement(video::VertexElement::EUsage::Diffuse, video::VertexElement::EType::Color);
	m_VertexFormat = video::VertexFormat("particle_point_renderer", decl);

	m_BaseMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleTransparent");
	m_EmitMaterial = video::MaterialLibrary::Instance()->CreateMaterial("particleEmit");
}

PointRendererMachine::~PointRendererMachine()
{
}

void PointRendererMachine::CreateBuffer(ParticleGroupData* group)
{
	if(m_Vertices.Size() < group->GetParticleCount())
		m_Vertices.Resize(group->GetParticleCount());
}

void PointRendererMachine::Render(video::Renderer* videoRenderer, ParticleGroupData* group, ParticleRenderer* renderer)
{
	if(group->GetParticleCount() == 0)
		return;

	auto data = dynamic_cast<PointRenderer*>(renderer);
	if(!data)
		return;

	CreateBuffer(group);

	ParticleModel* model = group->GetModel();
	u32 cursor = 0;
	for(auto& p : group->GetPool()) {
		video::Color color;
		float alpha = model->ReadValue(p, Particle::EParameter::Alpha);
		float red = model->ReadValue(p, Particle::EParameter::Red);
		float green = model->ReadValue(p, Particle::EParameter::Green);
		float blue = model->ReadValue(p, Particle::EParameter::Blue);
		color.SetF(alpha, red, green, blue);

		m_Vertices[cursor].position = p.position;
		m_Vertices[cursor].color = color;

		cursor += 1;
	}

	videoRenderer->SetMaterial(data->EmitLight ? m_EmitMaterial : m_BaseMaterial);
	videoRenderer->DrawPrimitiveList(
		video::EPrimitiveType::Points,
		group->GetParticleCount(),
		m_Vertices.Data_c(),
		group->GetParticleCount(),
		m_VertexFormat,
		true);
}

core::Name PointRendererMachine::GetType() const
{
	static const core::Name name("lux.particlerenderer.point");
	return name;
}

StrongRef<ParticleRenderer> PointRendererMachine::CreateRenderer()
{
	return LUX_NEW(PointRenderer)(this);
}

} // namespace scene
} // namespace lux
