#include "scene/particle/ParticleModel.h"

namespace lux
{
namespace scene
{

const float ParticleModel::DEFAULT[ParticleParam::EParameter::COUNT] = {
	1.0f, // Red
	1.0f, // Green
	1.0f, // Blue
	1.0f, // Alpha

	1.0f, // Size

	0.0f, // Angle
	0.0f, // RotSpeed

	0.0f, // Sprite

	0.0f, // EPP_CUSTOM_1
	0.0f, // EPP_CUSTOM_2
	0.0f // EPP_CUSTOM_3
};

ParticleModel::ParticleModel() :
	m_SmoothingModel(1),
	m_ParticleDataSize(0),
	m_ChangeId(0),
	m_ParamTypeChangeId(0)
{
	SetRGB(video::ColorF(1, 1, 1));
}

ParticleModel::~ParticleModel()
{
}

void ParticleModel::SetLifetime(const core::Distribution& time)
{
	m_Lifetime = time;
	++m_ChangeId;
}

const core::Distribution& ParticleModel::GetLifetime() const
{
	return m_Lifetime;
}

static int GetParticleUpdateValues(ParticleParam::EState state)
{
	switch(state) {
	case ParticleParam::EState::Disabled:
	case ParticleParam::EState::Constant:
	case ParticleParam::EState::Fixed:
	case ParticleParam::EState::Random:
	case ParticleParam::EState::Interpolated:
		return 0;
	case ParticleParam::EState::Changing:
	case ParticleParam::EState::ChangingRandom:
		return 1;
	default:
		return 0;
	}
}

void ParticleModel::SetParam(ParticleParam::EParameter param, const ParticleParam& value)
{
	int paramId = (int)param;
	if(m_Params[paramId].param.state != value.state)
		++m_ParamTypeChangeId;

	m_Params[paramId].param = value;
	for(int i = paramId; i < PARAM_COUNT; ++i) {
		m_Params[i].value_offset = (s8)(i ? m_Params[i - 1].value_offset + (m_Params[i - 1].param.IsEnabled() ? 1 : 0) : 0);
		m_Params[i].update_offset = (s8)(i ? m_Params[i - 1].update_offset + GetParticleUpdateValues(m_Params[i - 1].param.state) : 0);
	}

	m_ParticleDataSize = m_Params[PARAM_COUNT - 1].value_offset + m_Params[PARAM_COUNT - 1].update_offset;

	++m_ChangeId;
}

const ParticleParam& ParticleModel::GetParam(ParticleParam::EParameter param) const
{
	return GetInternalParam(param).param;
}

float ParticleModel::ReadValue(const Particle& p, ParticleParam::EParameter param) const
{
	int off = GetParamOffset(param);
	if(off != -1)
		return p.Param(off);
	else
		return DEFAULT[(int)param];
}

void ParticleModel::InitParticle(Particle& particle) const
{
	particle.age = 0;
	particle.life = m_Lifetime.Sample(m_Randomizer);

	const u8 end_values = m_Params[PARAM_COUNT - 1].value_offset;
	for(auto& p : m_Params) {
		switch(p.param.state) {
		case ParticleParam::EState::Fixed:
			particle.params[p.value_offset] = p.param.values[0];
			break;
		case ParticleParam::EState::Random:
			particle.params[p.value_offset] = m_Randomizer.GetFloat(p.param.values[0], p.param.values[1]);
			break;
		case ParticleParam::EState::Changing:
			particle.params[p.value_offset] = p.param.values[0];
			particle.params[end_values + p.update_offset] = (p.param.values[1] - p.param.values[0]) / particle.life;
			break;
		case ParticleParam::EState::ChangingRandom:
			particle.params[p.value_offset] = m_Randomizer.GetFloat(p.param.values[0], p.param.values[1]);
			particle.params[end_values + p.update_offset] = (m_Randomizer.GetFloat(p.param.values[0], p.param.values[1]) - particle.params[p.value_offset]) / particle.life;
			break;
		default:
			break;
		}
	}
}

void ParticleModel::UpdateParticle(Particle& particle, float secsPassed) const
{
	const u8 end_values = m_Params[PARAM_COUNT - 1].value_offset;
	for(auto& p : m_Params) {
		switch(p.param.state) {
		case ParticleParam::EState::Changing:
		case ParticleParam::EState::ChangingRandom:
			particle.Param(p.value_offset) += secsPassed * particle.Param(end_values + p.update_offset);
			break;
		case ParticleParam::EState::Interpolated:
			particle.Param(p.value_offset) = p.param.curve->Evaluate<float>(particle.age);
			break;
		default:
			break;
		}
	}
}

StrongRef<ParticleRenderer> ParticleModel::SetRenderMode(core::Name type)
{
	if(m_Renderer == nullptr || type != m_Renderer->GetReferableType())
		m_Renderer = core::ReferableFactory::Instance()->Create(type).AsStrong<ParticleRenderer>();
	return m_Renderer;
}

} // namespace scene
} // namespace lux
