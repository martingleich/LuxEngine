#ifndef INCLUDED_NORMALPARTICLEEMITTER_H
#define INCLUDED_NORMALPARTICLEEMITTER_H
#include "ParticleEmitter.h"

namespace lux
{
namespace scene
{

class NormalEmitter : public ParticleEmitter
{
public:
	NormalEmitter(float scatter=0.0f)
	{
		m_Scatter = scatter;
	}

	void SetScattering(float scatter)
	{
		m_Scatter = scatter;
	}

	core::Name GetReferableType() const
	{
		static const core::Name name("lux.emitter.Normal");
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(NormalEmitter)(*this);
	}

protected:
	void GenerateVelocity(Particle& particle, float speed) const
	{
		math::Vector3F scatter = m_Rand.GetVector3() * m_Scatter;
		math::Vector3F s = m_Zone->GetNormal(particle.position) + scatter;
		particle.velocity = speed * s;
	}

private:
	float m_Scatter;
};

} // namespace scene
} // namespace lux

#endif
