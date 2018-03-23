#ifndef INCLUDED_PARTICLE_SYSTEM_TEMPLATE_H
#define INCLUDED_PARTICLE_SYSTEM_TEMPLATE_H
#include "scene/particle/ParticleAffector.h"
#include "scene/particle/ParticleEmitter.h"
#include "scene/particle/ParticleModel.h"
#include "core/lxOrderedMap.h"
#include "core/ReferableFactory.h"

namespace lux
{
namespace scene
{
class ParticleSystemTemplate;
struct ParticleSystemTemplateBuilder
{
	core::Array<StrongRef<AbstractParticleAffector>> affectors;
	core::Array<StrongRef<AbstractParticleEmitter>> emitters;
	core::OrderedMap<ParticleModel*, int> capacities;
	bool isGlobal = true;

	StrongRef<ParticleModel> CreateModel()
	{
		return LUX_NEW(ParticleModel);
	}

	StrongRef<AbstractParticleEmitter> AddEmitter(AbstractParticleEmitter* emitter)
	{
		emitters.PushBack(emitter);
		return emitter;
	}
	StrongRef<AbstractParticleEmitter> AddEmitter(core::Name name)
	{
		return AddEmitter(core::ReferableFactory::Instance()->Create(name).As<ParticleEmitter>());
	}
	StrongRef<AbstractParticleAffector> AddAffector(AbstractParticleAffector* affector)
	{
		affectors.PushBack(affector);
		return affector;
	}
	StrongRef<AbstractParticleAffector> AddAffector(core::Name name)
	{
		return AddAffector(core::ReferableFactory::Instance()->Create(name).As<ParticleAffector>());
	}

	StrongRef<ParticleSystemTemplate> MakeTemplate();
};

class ParticleSystemTemplate : public Referable
{
	LX_REFERABLE_MEMBERS_API(ParticleSystemTemplate, LUX_API);
public:
	LUX_API ParticleSystemTemplate();
	LUX_API ParticleSystemTemplate(
		const core::Array<StrongRef<AbstractParticleAffector>>& affectors,
		const core::Array<StrongRef<AbstractParticleEmitter>>& emitters,
		const core::OrderedMap<ParticleModel*, int>& capacities,
		bool isGlobal);

	LUX_API int GetAffectorCount() const;
	LUX_API StrongRef<AbstractParticleAffector> GetAffector(int i);
	LUX_API StrongRef<AbstractParticleAffector>* GetAffectors();

	LUX_API int GetEmitterCount() const;
	LUX_API StrongRef<AbstractParticleEmitter> GetEmitter(int i);
	LUX_API StrongRef<AbstractParticleEmitter>* GetEmitters();

	LUX_API int GetModelCount();
	LUX_API StrongRef<ParticleModel> GetModel(int modelId);
	LUX_API int GetModelCapacity(int modelId);
	LUX_API int GetModelFirstAffector(int modelId);
	LUX_API int GetModelAffectorCount(int modelId);

	/**
	Global particles move in the global coordinate system, they don't move with the component.
	Non global particles move if the component is moved.
	Default is global.
	*/
	bool IsGlobal() const
	{
		return m_IsGlobal;
	}

private:
	// Sorted by group.
	core::Array<StrongRef<AbstractParticleAffector>> m_Affectors;
	core::Array<StrongRef<AbstractParticleEmitter>> m_Emitters;

	struct Group
	{
		StrongRef<ParticleModel> model;
		int capacity;
		int firstAffector;
		int affectorCount;
	};

	core::Array<Group> m_Groups;
	bool m_IsGlobal;
};

inline StrongRef<ParticleSystemTemplate> ParticleSystemTemplateBuilder::MakeTemplate()
{
	return LUX_NEW(ParticleSystemTemplate)(
		affectors,
		emitters,
		capacities,
		isGlobal);
}

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLESYSTEMTEMPLATE_H
