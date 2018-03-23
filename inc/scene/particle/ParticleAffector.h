#ifndef INCLUDED_LUX_PARTICLE_AFFECTOR_H
#define INCLUDED_LUX_PARTICLE_AFFECTOR_H
#include "scene/particle/ParticleModel.h"

#include "scene/Component.h"
#include "math/Transformation.h"

namespace lux
{
namespace scene
{

class AbstractParticleAffector : public Component
{
public:
	virtual ParticleModel* GetModel() const = 0;
	virtual void Begin(const math::Transformation& trans) = 0;
	virtual void Apply(Particle& particle, float secsPassed) = 0;
	StrongRef<AbstractParticleAffector> Clone() const
	{
		return CloneImpl().StaticCastStrong<AbstractParticleAffector>();
	}
};

class ParticleAffector : public AbstractParticleAffector
{
public:
	ParticleAffector() :
		m_Model(nullptr)
	{
	}

	void SetModel(ParticleModel* m) { m_Model = m; }
	ParticleModel* GetModel() const { return m_Model; }

	void SetTransform(const math::Transformation& transform)
	{
		m_Transform = transform;
	}
	const math::Transformation& GetTransform() const
	{
		return m_Transform;
	}

	virtual void Begin(const math::Transformation& trans)
	{
		m_AbsTransform = m_Transform.CombineRight(trans);
	}

	StrongRef<ParticleAffector> Clone() const
	{
		return CloneImpl().StaticCastStrong<ParticleAffector>();
	}

protected:
	StrongRef<ParticleModel> m_Model;
	math::Transformation m_Transform;
	math::Transformation m_AbsTransform;
};

}
}

#endif // #ifndef INCLUDED_LUX_PARTICLE_AFFECTOR_H