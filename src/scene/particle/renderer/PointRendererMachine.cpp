#include "scene/particle/renderer/PointRendererMachine.h"
#include "scene/particle/BuiltinParticleRenderers.h"
#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleGroupData.h"

#include "video/MaterialLibrary.h"

LX_REGISTER_REFERABLE_CLASS(lux::scene::PointRendererMachine, "lux.particlerenderermachine.Point");

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

	m_DefaultPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_DefaultPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_DefaultPass.alpha.blendOperator = video::EBlendOperator::Add;
	m_DefaultPass.zWriteEnabled = false;
	m_DefaultPass.fogEnabled = false;
	m_DefaultPass.lighting = video::ELightingFlag::Disabled;
	video::FixedFunctionParameters paramsDiffuse({}, {}, true);
	m_DefaultPass.shader = video::MaterialLibrary::Instance()->GetFixedFunctionShader(paramsDiffuse);

	m_EmitPass = m_DefaultPass;
	m_EmitPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_EmitPass.alpha.dstFactor = video::EBlendFactor::One;
	m_EmitPass.alpha.blendOperator = video::EBlendOperator::Add;
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
	int cursor = 0;
	for(auto& p : group->GetPool()) {
		video::Color color;
		float alpha = model->ReadValue(p, ParticleParam::Alpha);
		float red = model->ReadValue(p, ParticleParam::Red);
		float green = model->ReadValue(p, ParticleParam::Green);
		float blue = model->ReadValue(p, ParticleParam::Blue);
		color.SetF(alpha, red, green, blue);

		m_Vertices[cursor].position = p.position;
		m_Vertices[cursor].color = color;

		cursor += 1;
	}

	if(data->EmitLight)
		videoRenderer->SetPass(m_EmitPass, true);
	else
		videoRenderer->SetPass(m_DefaultPass, true);

	videoRenderer->Draw3DPrimitiveList(
		video::EPrimitiveType::Points,
		group->GetParticleCount(),
		m_Vertices.Data_c(),
		group->GetParticleCount(),
		m_VertexFormat);
}

core::Name PointRendererMachine::GetReferableType() const
{
	static const core::Name name("lux.particlerenderermachine.Point");
	return name;
}

} // namespace scene
} // namespace lux
