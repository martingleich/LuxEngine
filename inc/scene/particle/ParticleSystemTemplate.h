#ifndef INCLUDED_PARTICLE_SYSTEM_TEMPLATE_H
#define INCLUDED_PARTICLE_SYSTEM_TEMPLATE_H
#include "affector/ParticleAffector.h"
#include "emitter/ParticleEmitter.h"
#include "emitter/StraightEmitter.h"
#include "scene/particle/ParticleModel.h"

namespace lux
{
namespace scene
{

class ParticleSystemTemplate : public Referable
{
	LX_REFERABLE_MEMBERS_API(ParticleSystemTemplate, LUX_API);
public:
	virtual ~ParticleSystemTemplate() {}

	StrongRef<ParticleModel> AddModel()
	{
		return LUX_NEW(ParticleModel);
	}

	StrongRef<ParticleAffector> AddAffector(ParticleAffector* affector)
	{
		m_Affectors.PushBack(affector);
		return affector;
	}
	int GetAffectorCount() const
	{
		return m_Affectors.Size();
	}
	StrongRef<ParticleAffector> GetAffector(int i)
	{
		return m_Affectors.At(i);
	}
	void RemoveAffector(int i)
	{
		if(i >= m_Affectors.Size())
			throw core::OutOfRangeException();
		m_Affectors.Erase(m_Affectors.First() + i);
	}
	void ClearAffectors()
	{
		m_Affectors.Clear();
	}

	StrongRef<ParticleEmitter> AddEmitter(ParticleEmitter* emitter)
	{
		m_Emitters.PushBack(emitter);
		return emitter;
	}

	StrongRef<StraightEmitter> AddStraightEmitter(const math::Vector3F& dir)
	{
		auto out = LUX_NEW(StraightEmitter)(dir);
		AddEmitter(out);
		return out;
	}

	int GetEmitterCount() const
	{
		return m_Emitters.Size();
	}
	StrongRef<ParticleEmitter> GetEmitter(int i)
	{
		return m_Emitters.At(i);
	}
	void RemoveEmitter(int i)
	{
		if(i >= m_Emitters.Size())
			throw core::OutOfRangeException();
		m_Emitters.Erase(m_Emitters.First() + i);
	}
	void ClearEmitters()
	{
		m_Emitters.Clear();
	}

	void Finalize()
	{
	}

private:
	core::Array<StrongRef<ParticleAffector>> m_Affectors;
	core::Array<StrongRef<ParticleEmitter>> m_Emitters;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLESYSTEMTEMPLATE_H
