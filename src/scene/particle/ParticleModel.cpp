#include "scene/particle/ParticleModel.h"
#include "scene/particle/ParticleInterpolator.h"
#include "scene/particle/ParticleSystemManager.h"

namespace lux
{
namespace scene
{

const float ParticleModel::DEFAULT[(u32)Particle::EParameter::COUNT] = {
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
	m_LifeTimeMin(0.0f),
	m_LifeTimeMax(0.0f),
	m_IsImmortal(false),
	m_ParamCount(0),
	m_StaticCount(0),
	m_ChangingCount(0),
	m_InterpolatedCount(0),
	m_ParticleDataSize(0),
	m_Capacity(0)
{
}

void ParticleModel::SetParamStates(const ParticleParamStates& states)
{
	m_LifeTimeMin = 1.0f;
	m_LifeTimeMax = 1.0f;

	m_IsImmortal = false;

	m_StaticCount = 0;
	m_ChangingCount = 0;
	u8 cur = 0;
	const EParticleParamState* params = states.states;

	for(u32 i = 0; i < PARAM_COUNT; ++i) {
		m_Params[i].state = params[i];
		if(i < 3 && params[i] == EParticleParamState::Disabled)
			m_Params[i].state = EParticleParamState::Constant; // Force red, green and blue.

		// Force angle when rot-speed is given.
		if(i == (u32)Particle::EParameter::Angle) {
			if(states.states[(u32)Particle::EParameter::RotSpeed] != EParticleParamState::Disabled && m_Params[i].state == EParticleParamState::Disabled)
				m_Params[i].state = EParticleParamState::Fixed;
		}

		m_Params[i].param = Particle::EParameter(i);

		if(m_Params[i].state == EParticleParamState::Fixed || m_Params[i].state == EParticleParamState::Random) {
			++m_StaticCount;
		} else if(m_Params[i].state == EParticleParamState::Changing || m_Params[i].state == EParticleParamState::ChangingRandom) {
			m_BaseOffset[m_ChangingCount] = (u8)i;
			++m_ChangingCount;
		} else if(m_Params[i].state == EParticleParamState::Interpolated) {
			m_BaseOffset[PARAM_COUNT - m_InterpolatedCount - 1] = (u8)i;
			++m_InterpolatedCount;
		}

		if(m_Params[i].state != EParticleParamState::Disabled && m_Params[i].state != EParticleParamState::Constant) {
			m_Params[i].offset = cur;
			++cur;
		} else {
			m_Params[i].offset = -1;
		}
	}

	m_ParamCount = (u32)cur;
	m_ParticleDataSize = m_StaticCount * sizeof(float) + m_ChangingCount * sizeof(float) * 2 + m_InterpolatedCount * sizeof(float) * 2;

	ResetToDefault();
}

void ParticleModel::ResetToDefault()
{
	for(u32 i = 0; i < (size_t)Particle::EParameter::COUNT; ++i) {
		for(u32 j = 0; j < 4; ++j)
			m_Params[i].values[j] = DEFAULT[i];
	}
}

void ParticleModel::SetLifeTime(float min, float max)
{
	lxAssert(min <= max);

	m_LifeTimeMin = min;
	m_LifeTimeMax = max;
}

void ParticleModel::GetLifeTime(float& min, float& max)
{
	min = m_LifeTimeMin;
	max = m_LifeTimeMax;
}

void ParticleModel::SetImmortal(bool immortal)
{
	m_IsImmortal = immortal;
}

bool ParticleModel::IsImmortal() const
{
	return m_IsImmortal;
}

EParticleParamState ParticleModel::GetParamState(Particle::EParameter param) const
{
	return GetParam(param).state;
}

void ParticleModel::SetValues(Particle::EParameter param, const float* values)
{
	for(u32 i = 0; i < 4; ++i)
		GetParam(param).values[i] = values[i];
}

void ParticleModel::GetValues(Particle::EParameter param, float* values) const
{
	for(u32 i = 0; i < 4; ++i)
		values[i] = GetParam(param).values[i];
}

void ParticleModel::SetInterpolator(Particle::EParameter param, ParticleInterpolator* interpolator)
{
	GetParam(param).interpolator = interpolator;
}

StrongRef<ParticleInterpolator> ParticleModel::GetInterpolator(Particle::EParameter param) const
{
	if(GetParam(param).state == EParticleParamState::Interpolated)
		return GetParam(param).interpolator;
	else
		return nullptr;
}

int ParticleModel::GetOffset(Particle::EParameter param)
{
	return GetParam(param).offset;
}

float ParticleModel::GetDefaultValue(Particle::EParameter param) const
{
	return DEFAULT[(u32)param];
}

u32 ParticleModel::GetBytesParticleParams() const
{
	return m_ParticleDataSize;
}

u32 ParticleModel::GetFloatParticleParams() const
{
	return m_ParticleDataSize / sizeof(float);
}

void ParticleModel::InitParticle(Particle& particle) const
{
	particle.age = 0.0f;
	particle.life = m_Randomizer.GetFloat(m_LifeTimeMin, m_LifeTimeMax);
	particle.velocity = math::Vector3F::ZERO;

	float* p_val = particle.params;
	float* p_delta = p_val + m_StaticCount + m_ChangingCount;
	float* p_inter = p_delta + m_ChangingCount;

	for(u32 i = 0; i < (u32)Particle::EParameter::COUNT; ++i) {
		const Param& param = m_Params[i];

		float value = 0, delta = 0, scaleX = 1;
		bool is_delta = false, inter = false;
		switch(param.state) {
		case EParticleParamState::Disabled:
		case EParticleParamState::Constant:
			continue; // Abort for loop
		case EParticleParamState::Fixed:
			value = param.values[0];
			break;
		case EParticleParamState::Random:
			value = m_Randomizer.GetFloat(param.values[0], param.values[1]);
			break;

		case EParticleParamState::Changing:
			value = param.values[0];
			delta = (param.values[1] - value) / particle.life;
			is_delta = true;
			break;
		case EParticleParamState::ChangingRandom:
			value = m_Randomizer.GetFloat(param.values[0], param.values[1]);
			delta = m_Randomizer.GetFloat(param.values[2], param.values[3]) - value;
			is_delta = true;
			break;

		case EParticleParamState::Interpolated:
			auto interpolator = m_Params[i].interpolator;
			lxAssert(interpolator);
			value = interpolator->Interpolate(particle, 1.0f);
			scaleX = 1.0f;
			inter = true;
			break;
		}

		*p_val++ = value;
		if(is_delta) {
			is_delta = false;
			*p_delta++ = delta;
		}
		if(inter) {
			inter = false;
			*p_inter++ = scaleX;
		}
	}
}

void ParticleModel::UpdateParticle(Particle& particle, float secsPassed) const
{
	if(!m_IsImmortal) {
		const float* p_mut = particle.params + m_StaticCount + m_ChangingCount;
		for(u32 i = 0; i < m_ChangingCount; ++i) {
			const u32 off = m_Params[m_BaseOffset[i]].offset;
			particle.Param(off) += *p_mut * secsPassed;
			++p_mut;
		}
	}

	// Interpolation isn't based on percentage of used life
	// so it can be used on immortal particles
	const float* p_inter = particle.params + m_StaticCount + 2 * m_ChangingCount;
	for(u32 i = 0; i < m_InterpolatedCount; ++i) {
		const u32 index = m_BaseOffset[PARAM_COUNT - i - 1];
		const u32 offset = m_Params[index].offset;
		const float scaleX = *p_inter++;
		const ParticleInterpolator* inter = m_Params[index].interpolator;

		particle.Param(offset) = inter->Interpolate(particle, scaleX);
	}
}

void ParticleModel::SetGravity(const math::Vector3F& v)
{
	m_Gravity = v;
}

const math::Vector3F& ParticleModel::GetGravity() const
{
	return m_Gravity;
}

StrongRef<ParticleRenderer> ParticleModel::SetRenderer(ParticleRenderer* r)
{
	m_Renderer = r;
	return r;
}

StrongRef<ParticleRenderer> ParticleModel::GetRenderer()
{
	return m_Renderer;
}

StrongRef<ParticleRenderer> ParticleModel::SetRenderMode(core::Name type)
{
	if(m_Renderer == nullptr || type != m_Renderer->GetType())
		m_Renderer = ParticleSystemManager::Instance()->GetParticleRendererMachine(type)->CreateRenderer();

	return m_Renderer;
}

} // namespace scene
} // namespace lux
