#ifndef INCLUDED_AFFECTOR_LINEAR_FORCE_H
#define INCLUDED_AFFECTOR_LINEAR_FORCE_H
#include "ParticleAffector.h"

namespace lux
{
namespace scene
{

class LinearForceAffector : public ParticleAffector
{
public:
	LinearForceAffector() :
		m_Direction(math::vector3f::UNIT_Y)
	{}

	LinearForceAffector(const math::vector3f& force) :
		m_Direction(force)
	{
	}

	core::Name GetReferableType() const
	{
		static const core::Name name = "lux.affector.Linear";
		return name;
	}

	void Begin(const math::Transformation& trans)
	{
		m_TransDirection = trans.TransformDir(m_Direction);
	}

	void Apply(Particle& particle, float secsPassed)
	{
		particle.velocity += m_TransDirection*secsPassed;
	}

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(LinearForceAffector)(*this);
	}

private:
	math::vector3f m_Direction;
	math::vector3f m_TransDirection;
};

}
}

#endif // #ifndef INCLUDED_AFFECTOR_LINEAR_FORCE_H