#ifndef INCLUDED_PARTICLE_AFFECTOR_H
#define INCLUDED_PARTICLE_AFFECTOR_H
#include "scene/particle/Particle.h"
#include "scene/particle/ParticleModel.h"

#include "scene/Component.h"
#include "math/Transformation.h"

namespace lux
{
namespace scene
{

class ParticleAffector : public Component
{
public:
	ParticleAffector() :
		m_IsGlobal(false),
		m_Model(nullptr)
	{}

	virtual void Begin(const math::Transformation& trans) {
		LUX_UNUSED(trans);
	}
	virtual void Apply(Particle& particle, float secsPassed) = 0;

	bool IsGlobal() const
	{
		return m_IsGlobal;
	}

	void SetGlobal(bool isGlobal)
	{
		m_IsGlobal = isGlobal;
	}

	void SetModel(ParticleModel* m)
	{
		m_Model = m;
	}
	ParticleModel* GetModel() const
	{
		return m_Model;
	}

	StrongRef<ParticleAffector> Clone() const
	{
		return CloneImpl().StaticCastStrong<ParticleAffector>();
	}

private:
	bool m_IsGlobal;
	StrongRef<ParticleModel> m_Model;
};

}
}

#endif // #ifndef INCLUDED_PARTICLE_AFFECTOR_H