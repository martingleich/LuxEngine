#ifndef INCLUDED_PARTICLEEMITTER_H
#define INCLUDED_PARTICLEEMITTER_H
#include "core/lxRandom.h"
#include "math/Transformation.h"

#include "scene/particle/ParticleModel.h"
#include "scene/Component.h"
#include "scene/Zone.h"

namespace lux
{
namespace scene
{

class AbstractParticleEmitter : public Component
{
public:
	virtual ParticleModel* GetModel() const = 0;
	virtual float GetFlow() const = 0;
	virtual int GetEmitCount(float secsPassed) = 0;
	virtual void Begin(const math::Transformation& transform) = 0;
	virtual void Emit(core::Randomizer& rand, Particle& particle) = 0;
	StrongRef<AbstractParticleEmitter> Clone() const
	{
		return CloneImpl().StaticCastStrong<AbstractParticleEmitter>();
	}
};

class ParticleEmitter : public AbstractParticleEmitter
{
public:
	ParticleEmitter() :
		m_Flow(0),
		m_Force(core::Distribution::Fixed(0)),
		m_Count(0)
	{
	}

	ParticleEmitter(const ParticleEmitter& other) :
		m_Flow(other.m_Flow),
		m_Force(other.m_Force),
		m_Count(0),
		m_Model(other.m_Model)
	{
		if(other.m_Zone)
			m_Zone = other.m_Zone->Clone();
		else
			m_Zone = nullptr;
	}

	void SetZone(Zone* zone) { m_Zone = zone; }
	StrongRef<Zone> GetZone() { return m_Zone; }

	void SetFlow(float flow) { m_Flow = flow; }
	float GetFlow() const { return m_Flow; }

	void SetForce(float force) { m_Force = core::Distribution::Fixed(force); }
	void SetForce(const core::Distribution& force) { m_Force = force; }

	const core::Distribution& GetForce() const { return m_Force; }

	void SetModel(ParticleModel* m) { m_Model = m; }
	ParticleModel* GetModel() const { return m_Model; }

	void SetTransform(const math::Transformation& transform)
	{
		m_Transform = transform;
	}
	void SetPosition(const math::Vector3F& pos)
	{
		m_Transform.translation = pos;
	}
	const math::Transformation& GetTransform() const
	{
		return m_Transform;
	}

	int GetEmitCount(float secsPassed)
	{
		m_Count += m_Flow * secsPassed;
		if(m_Count > 0.0f) {
			int out = (int)m_Count;
			m_Count -= std::floor(m_Count);
			return out;
		} else {
			return 0;
		}
	}

	void Begin(const math::Transformation& transform)
	{
		m_AbsTransform = m_Transform.CombineRight(transform);
	}
	void Emit(core::Randomizer& rand, Particle& particle)
	{
		if(m_Zone)
			particle.position = m_Zone->GetPointInside(rand);
		else
			particle.position = math::Vector3F::ZERO;

		GenerateVelocity(rand, particle, m_Force.Sample(rand));

		particle.position = m_AbsTransform.TransformPoint(particle.position);
		particle.velocity = m_AbsTransform.TransformDir(particle.velocity);
	}
	virtual void GenerateVelocity(core::Randomizer& rand, Particle& particle, float speed) = 0;

	StrongRef<ParticleEmitter> Clone() const
	{
		return CloneImpl().StaticCastStrong<ParticleEmitter>();
	}

protected:
	StrongRef<Zone> m_Zone;
	StrongRef<ParticleModel> m_Model;

	math::Transformation m_Transform;
	math::Transformation m_AbsTransform;

	float m_Flow;

	core::Distribution m_Force;

	float m_Count;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_IPARTICLEEMITTER
