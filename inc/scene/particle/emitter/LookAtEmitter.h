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
	LookAtEmitter() :
		m_Scatter(0.0f)
	{
	}

	LookAtEmitter(const math::vector3f& point, float scatter=0.0f)
	{
		SetPoint(point);
		m_Scatter = scatter;
	}

	void SetPoint(const math::vector3f& point)
	{
		m_Point = point;
	}

	void SetScattering(float scatter)
	{
		m_Scatter = scatter;
	}

	const math::vector3f& GetPoint() const
	{
		return m_Point;
	}

	core::Name GetReferableType() const
	{
		static const core::Name name = "lux.emitter.LookAt";
		return name;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(LookAtEmitter)(*this);
	}

	void Begin(const math::Transformation& trans) const
	{
		ParticleEmitter::Begin(trans);

		m_TransformedPoint = trans.TransformPoint(m_Point);
	}

protected:
	void GenerateVelocity(Particle& particle, float speed) const
	{
		math::vector3f scatter = m_Rand.GetVector3() * m_Scatter;
		math::vector3f dir = (m_TransformedPoint - particle.position).Normal_s();
		math::vector3f s = scatter - dir.Dot(scatter)*dir;
		particle.velocity = speed * (dir + s);
	}

private:
	math::vector3f m_Point;
	float m_Scatter;

	mutable math::vector3f m_TransformedPoint;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_ISTRAIGHTPARTICLEEMITTER
