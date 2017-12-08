#ifndef INCLUDED_LOOKATPARTICLEEMITTER_H
#define INCLUDED_LOOKATPARTICLEEMITTER_H
#include "ParticleEmitter.h"

namespace lux
{
namespace scene
{

class LookAtEmitter : public ParticleEmitter
{
public:
	LX_REFERABLE_MEMBERS_API(LUX_API);

	LookAtEmitter() :
		m_Scatter(0.0f)
	{
	}

	LookAtEmitter(const math::Vector3F& point, float scatter=0.0f)
	{
		SetPoint(point);
		m_Scatter = scatter;
	}

	void SetPoint(const math::Vector3F& point)
	{
		m_Point = point;
	}

	void SetScattering(float scatter)
	{
		m_Scatter = scatter;
	}

	const math::Vector3F& GetPoint() const
	{
		return m_Point;
	}

	void Begin(const math::Transformation& trans) const
	{
		ParticleEmitter::Begin(trans);

		m_TransformedPoint = trans.TransformPoint(m_Point);
	}

protected:
	void GenerateVelocity(Particle& particle, float speed) const
	{
		math::Vector3F scatter = m_Rand.GetVector3() * m_Scatter;
		math::Vector3F dir = (m_TransformedPoint - particle.position).Normal();
		math::Vector3F s = scatter - dir.Dot(scatter)*dir;
		particle.velocity = speed * (dir + s);
	}

private:
	math::Vector3F m_Point;
	float m_Scatter;

	mutable math::Vector3F m_TransformedPoint;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_ISTRAIGHTPARTICLEEMITTER
