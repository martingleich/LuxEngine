#ifndef INCLUDED_PARTICLE_SYSTEM_MANAGER_H
#define INCLUDED_PARTICLE_SYSTEM_MANAGER_H
#include "video/VideoDriver.h"
#include "video/MaterialLibrary.h"
#include "ParticleRenderer.h"
#include "core/lxHashMap.h"

#include "emitter/ParticleEmitter.h"
#include "affector/ParticleAffector.h"

namespace lux
{
namespace scene
{
class ParticleSystemTemplate;

class ParticleSystemManager : public ReferenceCounted
{
public:
	LUX_API static void Initialize();
	LUX_API static ParticleSystemManager* Instance();
	LUX_API static void Destroy();

	LUX_API ParticleSystemManager();
	LUX_API ~ParticleSystemManager();

	LUX_API void AddParticleRendererMachine(RendererMachine* m);
	LUX_API StrongRef<RendererMachine> GetParticleRendererMachine(core::Name n);

	LUX_API core::Range<core::HashMap<core::Name, StrongRef<RendererMachine>>::ConstKeyIterator> Machines() const;

	LUX_API StrongRef<ParticleSystemTemplate> CreateTemplate();

private:
	core::HashMap<core::Name, StrongRef<RendererMachine>> mMachines;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_PARTICLE_SYSTEM_MANAGER_H