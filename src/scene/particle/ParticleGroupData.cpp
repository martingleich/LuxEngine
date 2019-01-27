#include "scene/particle/ParticleGroupData.h"

#include "scene/particle/ParticleEmitter.h"
#include "scene/particle/ParticleAffector.h"

#include "scene/Node.h"

namespace lux
{
namespace scene
{

ParticleGroupData::ParticleGroupData(ParticleModel* model, int capacity) :
	m_Model(model),
	m_Pool(capacity)
{
	m_Data.Resize(m_Model->GetFloatParticleParams() * capacity);
	m_ModelChangeId = m_Model->GetParamStateChangeId();
}

ParticleGroupData::~ParticleGroupData()
{
}

void ParticleGroupData::AddParticle(int count, const math::Vector3F& position, const math::Vector3F& velocity)
{
	CreationData data;
	data.count = count;
	data.position = position;
	data.velocity = velocity;
	m_CreationData.PushBack(data);
}

void ParticleGroupData::Update(float secsPassed, const SystemData& data)
{
	if(m_Model->GetParamStateChangeId() != m_ModelChangeId) {
		// Full reset necessary.
		m_Data.Resize(m_Model->GetFloatParticleParams() * m_Data.Size());
		for(auto& p : m_Pool)
			p.params = nullptr;
		m_Pool.DisableAll();
		m_ModelChangeId = m_Model->GetParamStateChangeId();
	}

	if(data.emitterCount > m_EmitTransform.Size()) {
		m_EmitTransform.Resize(data.emitterCount);
		m_LastEmitTransform.Resize(data.emitterCount);
	}

	m_RotSpeedOffset = m_Model->GetParamOffset(ParticleParam::RotSpeed);
	m_AngleOffset = m_Model->GetParamOffset(ParticleParam::Angle);

	m_CurEmitId = 0;

	// Particle system coordinate system
	const math::Transformation& psTrans = data.psSystem->GetAbsoluteTransform();
	math::Transformation psTransI = psTrans.GetInverted();

	for(int i = 0; i < data.emitterCount; ++i) {
		auto& emitter = data.emitters[i];
		int number = emitter->GetEmitCount(secsPassed);
		if(number > 0) {
			if(m_Model->GetSmoothingModel() > 0) {
				m_LastEmitTransform[i] = m_EmitTransform[i];
				if(emitter->GetNode())
					m_EmitTransform[i] = emitter->GetNode()->GetAbsoluteTransform();
				else
					m_EmitTransform[i] = psTrans;
			} else {
				emitter->Begin(m_EmitTransform[i]);
			}
			m_EmitData.PushBack(EmitData(number, i));
		}
	}

	for(int i = 0; i < 2; ++i) {
		for(int j = 0; j < data.affectorsCounts[i]; ++j) {
			auto& affector = data.affectors[i][j];
			if(affector->GetNode())
				affector->Begin(affector->GetNode()->GetAbsoluteTransform());
			else
				affector->Begin(psTrans);
		}
	}

	// Animate particles
	math::Vector3F gravity = m_Model->GetGravity();
	for(core::Pool<Particle>::Iterator it = m_Pool.First(); it != m_Pool.End();) {
		if(AnimateParticle(*it, secsPassed, data)) {
			// If the particle was destroyed, reuse it for another emitter
			if(!LaunchParticle(*it, secsPassed, data))
				m_Pool.Disable(it);
			else
				++it;
		} else {
			it->velocity += gravity * secsPassed;
			++it;
		}
	}

	// If there still is more room, emit more particles
	int inactiveCount = m_Pool.GetInactiveCount();
	Particle* particle = nullptr;
	while(inactiveCount > 0 && (m_CreationData.Size() > 0 || m_EmitData.Size() > 0)) {
		particle = m_Pool.MakeActive();
		if(!particle)
			break;
		if(particle->params == nullptr)
			particle->params = &m_Data[(m_Pool.GetActiveCount() - 1)*m_Model->GetFloatParticleParams()];

		m_Model->InitParticle(*particle);
		if(!LaunchParticle(*particle, secsPassed, data)) {
			m_Pool.Disable(particle);
			break;
		}

		--inactiveCount;
	}

	// The remaining ones had no luck
	m_EmitData.Clear();
	m_CreationData.Clear();
}

bool ParticleGroupData::LaunchParticle(Particle& particle, float secsPassed, const SystemData& data)
{
	if(m_CreationData.Size() > 0) {
		m_Model->InitParticle(particle);

		CreationData& creator = m_CreationData.Back();
		particle.position = creator.position;
		particle.velocity = creator.velocity;

		creator.count -= 1;
		if(creator.count == 0)
			m_CreationData.PopBack();
	} else if(m_EmitData.Size() > 0) {
		m_Model->InitParticle(particle);
		int smoothingModel = m_Model->GetSmoothingModel();

		EmitData& emit = m_EmitData.Back()/*[m_CurEmitId]*/;
		if(smoothingModel >= 1) {
			//float inter = m_Random.GetFloat(0.0f, 1.0f);
			float inter = (float)emit.count / (float)emit.total;
			auto& lastTransform = m_LastEmitTransform[emit.emitter];
			auto& curTransform = m_EmitTransform[emit.emitter];
			math::Transformation t(
				math::Lerp(lastTransform.translation, curTransform.translation, inter),
				math::Lerp(lastTransform.orientation, curTransform.orientation, inter).Normalize(), // Not using Slerp here, since the angle between the quaternions is really small, in normal cases
				math::Lerp(lastTransform.scale, curTransform.scale, inter)); // Not using scale interpolation here, since the distance is really small

			data.emitters[emit.emitter]->Begin(t);
			data.emitters[emit.emitter]->Emit(m_Random, particle);
			float dt = (1 - inter)*secsPassed;
			if(smoothingModel >= 2) {
				particle.position += dt*(particle.velocity + m_Model->GetGravity()*dt);
				particle.velocity += dt * m_Model->GetGravity();
			} else {
				particle.position += dt*particle.velocity;
			}
		} else {
			data.emitters[emit.emitter]->Emit(m_Random, particle);
		}

		emit.count -= 1;
		if(emit.count == 0)
			m_EmitData.Resize(m_EmitData.Size() - 1);
	} else {
		return false;
	}

	return true;
}

bool ParticleGroupData::AnimateParticle(Particle& particle, float secsPassed, const SystemData& data)
{
	particle.age += secsPassed;

	m_Model->UpdateParticle(particle, secsPassed);

	particle.position += particle.velocity * secsPassed;

	if(m_RotSpeedOffset != -1 && m_AngleOffset != -1)
		particle.Param(m_AngleOffset) += particle.Param(m_RotSpeedOffset) * secsPassed;

	if(!m_Model->GetLifetime().IsInfinite())
		particle.life -= secsPassed;

	for(int i = 0; i < 2; ++i) {
		for(int j = 0; j < data.affectorsCounts[i]; ++j) {
			auto affector = data.affectors[i][j];
			affector->Apply(particle, secsPassed);
			if(particle.life <= 0)
				return true;
		}
	}

	return (particle.life <= 0.0f);
}

}
}
