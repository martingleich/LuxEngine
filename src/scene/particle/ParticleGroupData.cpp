#include "scene/particle/ParticleGroupData.h"
#include "scene/particle/ParticleModel.h"

#include "scene/particle/emitter/ParticleEmitter.h"
#include "scene/particle/affector/ParticleAffector.h"

#include "scene/Node.h"

#define PARTICLE_GROUP_SMOOTH_FLOW

namespace lux
{
namespace scene
{

ParticleGroupData::ParticleGroupData(ParticleModel* model, u32 capacity) :
	m_Model(model),
	m_Pool(capacity)
{
	m_Data.Resize(m_Model->GetFloatParticleParams() * capacity);
}

ParticleGroupData::~ParticleGroupData()
{
}

const core::Pool<Particle>& ParticleGroupData::GetPool() const
{
	return m_Pool;
}

bool ParticleGroupData::AnimateParticle(Particle& particle, float secsPassed, const SystemData& data)
{
	particle.age += secsPassed;

	m_Model->UpdateParticle(particle, secsPassed);

	particle.position += particle.velocity * secsPassed;

	if(m_RotSpeedOffset != -1 && m_AngleOffset != -1)
		particle.Param(m_AngleOffset) += particle.Param(m_RotSpeedOffset) * secsPassed;

	if(!m_Model->IsImmortal())
		particle.life -= secsPassed;

	for(int i = 0; i < data.globalCount; ++i) {
		auto affector = data.globals[i];
		affector->Apply(particle, secsPassed);
		if(particle.life <= 0.0f)
			return true;
	}

	for(int i = 0; i < data.localCount; ++i) {
		auto affector = data.locals[i];
		affector->Apply(particle, secsPassed);
		if(particle.life <= 0.0f)
			return true;
	}

	return (particle.life <= 0.0f);
}

bool ParticleGroupData::LaunchParticle(Particle& particle, float secsPassed)
{
	if(m_CreationData.Size() > 0) {
		m_Model->InitParticle(particle);

		CreationData& creator = *m_CreationData.Last();
		if(creator.emitter) {
			creator.emitter->Begin(math::Transformation::DEFAULT);
			creator.emitter->Emit(particle);
		} else {
			particle.position = creator.position;
			particle.velocity = creator.velocity;
		}

		creator.count -= 1;
		if(creator.count == 0)
			m_CreationData.Resize(m_CreationData.Size() - 1);
	} else if(m_EmitData.Size() > 0) {
		m_Model->InitParticle(particle);

		EmitData& emit = m_EmitData.Back()/*[m_CurEmitId]*/;
#ifdef PARTICLE_GROUP_SMOOTH_FLOW
		//float inter = m_Random.GetFloat(0.0f, 1.0f);
		float inter = (float)emit.count / (float)emit.total;
		math::Transformation t(
			math::Lerp(emit.entry->lastTransform.translation, emit.entry->transform.translation, inter),
			math::Lerp(emit.entry->lastTransform.orientation, emit.entry->transform.orientation, inter).Normalize(), // Not using Slerp here, since the angle between the quaternions is really small, in normal cases
			math::Lerp(emit.entry->lastTransform.scale, emit.entry->transform.scale, inter)); // Not using scale interpolation here, since the distance is really small

		emit.entry->e->Begin(t);
		emit.entry->e->Emit(particle);
		particle.position += ((1.0f) - inter)*secsPassed*particle.velocity;
#else
		emit.entry->e->Emit(particle);
#endif

		emit.count -= 1;
		if(emit.count == 0)
			m_EmitData.Resize(m_EmitData.Size() - 1);
	} else {
		return false;
	}

	return true;
}

void ParticleGroupData::AddParticle(u32 count, const math::Vector3F& position, const math::Vector3F& velocity)
{
	CreationData Data;
	Data.count = count;
	Data.position = position;
	Data.velocity = velocity;
	Data.emitter = nullptr;
	m_CreationData.PushBack(Data);
}

void ParticleGroupData::AddParticle(u32 count, ParticleEmitter* emitter)
{
	CreationData Data;
	Data.count = count;
	Data.emitter = emitter;
	m_CreationData.PushBack(Data);
}

void ParticleGroupData::Update(float secsPassed, const SystemData& data)
{
	m_RotSpeedOffset = m_Model->GetOffset(Particle::EParameter::RotSpeed);
	m_AngleOffset = m_Model->GetOffset(Particle::EParameter::Angle);

	m_CurEmitId = 0;

	// Particle system coordinate system
	const math::Transformation& psTrans = data.psSystem->GetAbsoluteTransform();
	math::Transformation psTransI = psTrans.GetInverted();

	for(int i = 0; i < data.emitterCount; ++i) {
		EmitterEntry& emitter = data.emitters[i];
		u32 number = emitter->GetEmitCount(secsPassed);
		if(number > 0) {
#ifndef PARTICLE_GROUP_SMOOTH_FLOW
			emitter->Begin(emitter.transform);
#endif
			m_EmitData.PushBack(EmitData(number, &emitter));
		}
	}

	for(int i = 0; i < data.globalCount; ++i) {
		auto affector = data.globals[i];
		const math::Transformation& ownerTrans = affector.n ?
			affector.n->GetAbsoluteTransform() :
			data.owner->GetAbsoluteTransform();
		if(affector->IsGlobal())
			affector->Begin(psTransI);
		else
			affector->Begin(ownerTrans.CombineLeft(psTransI));
	}

	for(int i = 0; i < data.localCount; ++i) {
		auto affector = data.locals[i];
		const math::Transformation& ownerTrans = affector.n ?
			affector.n->GetAbsoluteTransform() :
			data.owner->GetAbsoluteTransform();
		if(affector->IsGlobal())
			affector->Begin(psTransI);
		else
			affector->Begin(ownerTrans.CombineLeft(psTransI));
	}

	// Animate particles
	math::Vector3F gravity = m_Model->GetGravity();
	for(core::Pool<Particle>::Iterator it = m_Pool.First(); it != m_Pool.End();) {
		if(AnimateParticle(*it, secsPassed, data)) {
			// If the particle was destroyed, reuse it for another emitter
			if(!LaunchParticle(*it, secsPassed))
				m_Pool.Disable(it);
			else
				++it;
		} else {
			it->velocity += gravity * secsPassed;
			++it;
		}
	}

	// If there still is more room, emit more particles
	u32 inactiveCount = m_Pool.GetInactiveCount();
	Particle* particle = nullptr;
	while(inactiveCount > 0 && (m_CreationData.Size() > 0 || m_EmitData.Size() > 0)) {
		particle = m_Pool.MakeActive();
		if(!particle)
			break;
		if(particle->params == nullptr)
			particle->params = &m_Data[(m_Pool.GetActiveCount() - 1)*m_Model->GetFloatParticleParams()];

		m_Model->InitParticle(*particle);
		if(!LaunchParticle(*particle, secsPassed)) {
			m_Pool.Disable(particle);
			break;
		}

		--inactiveCount;
	}

	// The remaining ones had no luck
	m_EmitData.Clear();
	m_CreationData.Clear();
}

int ParticleGroupData::GetParticleCount() const
{
	return m_Pool.GetActiveCount();
}

int ParticleGroupData::GetParticleCapacity() const
{
	return m_Pool.Capactity();
}

ParticleModel* ParticleGroupData::GetModel()
{
	return m_Model;
}

}
}
