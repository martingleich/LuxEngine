#include "scene/particle/renderer/LineRendererMachine.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"
#include "video/MaterialLibrary.h"

namespace lux
{
namespace scene
{

static WeakRef<LineRendererMachine> g_SharedInstance;
StrongRef<LineRendererMachine> LineRendererMachine::GetShared()
{
	if(g_SharedInstance)
		return g_SharedInstance.GetStrong();
	StrongRef<LineRendererMachine> out = LUX_NEW(LineRendererMachine);
	g_SharedInstance = out.GetWeak();
	return out;
}
LineRendererMachine::LineRendererMachine()
{
	video::VertexFormatBuilder builder;
	builder.AddElement(video::VertexElement::EUsage::Position, video::VertexElement::EType::Float3);
	builder.AddElement(video::VertexElement::EUsage::Diffuse, video::VertexElement::EType::Color);
	m_VertexFormat = builder.Build("particleLine");

	m_DefaultPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_DefaultPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_DefaultPass.alpha.blendOperator = video::EBlendOperator::Add;
	m_DefaultPass.zWriteEnabled = false;
	m_DefaultPass.fogEnabled = false;
	m_DefaultPass.lighting = video::ELightingFlag::Disabled;
	m_DefaultPass.shader = video::ShaderFactory::Instance()->
		GetFixedFunctionShader(video::FixedFunctionParameters::VertexColorOnly());

	m_EmitPass = m_DefaultPass;
	m_EmitPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_EmitPass.alpha.dstFactor = video::EBlendFactor::One;
	m_EmitPass.alpha.blendOperator = video::EBlendOperator::Add;
}

void LineRendererMachine::CreateBuffer(ParticleGroupData* group)
{
	if(m_Vertices.Size() < group->GetParticleCount() * 2)
		m_Vertices.Resize(group->GetParticleCount() * 2);
}

void LineRendererMachine::Render(video::Renderer* videoRenderer, ParticleGroupData* group, LineRenderer* renderer)
{
	if(group->GetParticleCount() == 0)
		return;

	m_Data = renderer;

	CreateBuffer(group);

	ParticleModel* model = group->GetModel();
	int cursor = 0;
	for(auto& p : group->GetPool()) {
		float alpha = model->ReadValue(p, ParticleParam::Alpha);
		float red = model->ReadValue(p, ParticleParam::Red);
		float green = model->ReadValue(p, ParticleParam::Green);
		float blue = model->ReadValue(p, ParticleParam::Blue);

		video::Color color;
		color.SetF(alpha, red, green, blue);

		float lSq = p.velocity.GetLengthSq();
		auto delta = p.velocity;
		if(math::IsZero(lSq))
			delta = m_Data->DefaultDir;
		else if(!m_Data->ScaleSpeed)
			delta /= std::sqrt(lSq);
		delta *= m_Data->Length * model->ReadValue(p, ParticleParam::Size);

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

	videoRenderer->Draw(video::RenderRequest::FromMemory3D(
		video::EPrimitiveType::Lines, group->GetParticleCount(),
		m_Vertices.Data_c(), group->GetParticleCount() * 2, m_VertexFormat));
}

} // namespace scene
} // namespace lux