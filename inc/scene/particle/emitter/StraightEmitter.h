#ifndef INCLUDED_STRAIGHTPARTICLEEMITTER_H
#define INCLUDED_STRAIGHTPARTICLEEMITTER_H
#include "ParticleEmitter.h"

namespace lux
{
namespace scene
{

class StraightEmitter : public ParticleEmitter
{
public:
	StraightEmitter() :
		m_Direction(math::vector3f::UNIT_X),
		m_Scatter(0.0f)
	{
	}

	StraightEmitter(const math::vector3f& dir, float scatter=0.0f)
	{
		SetDirection(dir);
		m_Scatter = scatter;
	}

	void SetDirection(const math::vector3f& dir)
	{
		m_Direction = dir;
		m_Direction.Normalize_s();
	}

	void SetScattering(float scatter)
	{
		m_Scatter = scatter;
	}

	const math::vector3f& GetDirection() const
	{
		return m_Direction;
	}

	core::Name GetReferableType() const
	{
		static const core::Name name = "lux.emitter.Straight";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(StraightEmitter)(*this);
	}

	void Begin(const math::Transformation& trans) const
	{
		ParticleEmitter::Begin(trans);

		m_TransformedDirection = trans.TransformDir(m_Direction);
	}

protected:
	void GenerateVelocity(Particle& particle, float speed) const
	{
		math::vector3f scatter = m_Rand.GetVector3() * m_Scatter;
		math::vector3f s = scatter - m_TransformedDirection.Dot(scatter)*m_TransformedDirection;
		particle.velocity = speed * (m_TransformedDirection + s);
	}

private:
	math::vector3f m_Direction;
	float m_Scatter;

	mutable math::vector3f m_TransformedDirection;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_ISTRAIGHTPARTICLEEMITTER
