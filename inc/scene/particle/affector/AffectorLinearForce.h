#ifndef INCLUDED_AFFECTOR_LINEAR_FORCE_H
#define INCLUDED_AFFECTOR_LINEAR_FORCE_H
#include "ParticleAffector.h"

namespace lux
{
namespace scene
{

class LinearForceAffector : public ParticleAffector
{
	LX_REFERABLE_MEMBERS_API(LinearForceAffector, LUX_API);
public:
	LinearForceAffector() :
		m_Direction(math::Vector3F::UNIT_Y)
	{}

	LinearForceAffector(const math::Vector3F& force) :
		m_Direction(force)
	{
	}

	void Begin(const math::Transformation& trans)
	{
		m_TransDirection = trans.TransformDir(m_Direction);
	}

	void Apply(Particle& particle, float secsPassed)
	{
		particle.velocity += m_TransDirection*secsPassed;
	}

private:
	math::Vector3F m_Direction;
	math::Vector3F m_TransDirection;
};

}
}

#endif // #ifndef INCLUDED_AFFECTOR_LINEAR_FORCE_H