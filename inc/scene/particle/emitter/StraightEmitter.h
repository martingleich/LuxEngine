#ifndef INCLUDED_STRAIGHTPARTICLEEMITTER_H
#define INCLUDED_STRAIGHTPARTICLEEMITTER_H
#include "ParticleEmitter.h"

namespace lux
{
namespace scene
{

class StraightEmitter : public ParticleEmitter
{
	LX_REFERABLE_MEMBERS_API(StraightEmitter, LUX_API);
public:
	StraightEmitter() :
		m_Direction(math::Vector3F::UNIT_X),
		m_Scatter(0.0f)
	{
	}

	StraightEmitter(const math::Vector3F& dir, float scatter=0.0f)
	{
		SetDirection(dir);
		m_Scatter = scatter;
	}

	void SetDirection(const math::Vector3F& dir)
	{
		m_Direction = dir;
		m_Direction.Normalize();
	}

	void SetScattering(float scatter)
	{
		m_Scatter = scatter;
	}

	const math::Vector3F& GetDirection() const
	{
		return m_Direction;
	}

	void Begin(const math::Transformation& trans) const
	{
		ParticleEmitter::Begin(trans);

		m_TransformedDirection = trans.TransformDir(m_Direction);
	}

protected:
	void GenerateVelocity(Particle& particle, float speed) const
	{
		math::Vector3F scatter = m_Rand.GetVector3() * m_Scatter;
		math::Vector3F s = scatter - m_TransformedDirection.Dot(scatter)*m_TransformedDirection;
		particle.velocity = speed * (m_TransformedDirection + s);
	}

private:
	math::Vector3F m_Direction;
	float m_Scatter;

	mutable math::Vector3F m_TransformedDirection;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_ISTRAIGHTPARTICLEEMITTER
