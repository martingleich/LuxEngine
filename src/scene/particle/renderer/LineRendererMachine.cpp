#include "LineRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"
#include "video/MaterialLibrary.h"

LX_REGISTER_REFERABLE_CLASS(lux::scene::LineRendererMachine, "lux.particlerenderer.Line");

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

	m_DefaultPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_DefaultPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_DefaultPass.alpha.blendOperator = video::EBlendOperator::Add;
	m_DefaultPass.zWriteEnabled = false;
	m_DefaultPass.fogEnabled = false;
	m_DefaultPass.lighting = video::ELighting::Disabled;
	video::FixedFunctionParameters paramsDiffuse({}, {}, true);
	m_DefaultPass.shader = video::MaterialLibrary::Instance()->GetFixedFunctionShader(paramsDiffuse);

	m_EmitPass = m_DefaultPass;
	m_EmitPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_EmitPass.alpha.dstFactor = video::EBlendFactor::One;
	m_EmitPass.alpha.blendOperator = video::EBlendOperator::Add;
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
		videoRenderer->SetPass(m_EmitPass, true);
	else
		videoRenderer->SetPass(m_DefaultPass, true);

	videoRenderer->Draw3DPrimitiveList(
		video::EPrimitiveType::Lines,
		group->GetParticleCount(),
		m_Vertices.Data_c(),
		group->GetParticleCount() * 2,
		m_VertexFormat);
}

StrongRef<ParticleRenderer> LineRendererMachine::CreateRenderer()
{
	return LUX_NEW(LineRenderer)(this);
}

core::Name LineRendererMachine::GetReferableType() const
{
	static const core::Name name("lux.particlerenderer.Line");
	return name;
}

} // namespace scene
} // namespace lux