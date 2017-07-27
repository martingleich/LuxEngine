#include "scene/particle/ParticleSystemManager.h"

#include "scene/particle/renderer/QuadRendererMachine.h"
#include "scene/particle/renderer/LineRendererMachine.h"
#include "scene/particle/renderer/PointRendererMachine.h"

#include "scene/particle/ParticleSystemTemplate.h"

namespace lux
{
namespace scene
{
static StrongRef<ParticleSystemManager> g_Manager;

void ParticleSystemManager::Initialize()
{
	g_Manager = LUX_NEW(ParticleSystemManager);
}

ParticleSystemManager* ParticleSystemManager::Instance()
{
	return g_Manager;
}

void ParticleSystemManager::Destroy()
{
	g_Manager.Reset();
}

ParticleSystemManager::ParticleSystemManager()
{
	AddParticleRendererMachine(LUX_NEW(QuadRendererMachine));
	AddParticleRendererMachine(LUX_NEW(LineRendererMachine));
	AddParticleRendererMachine(LUX_NEW(PointRendererMachine));
}

ParticleSystemManager::~ParticleSystemManager()
{
}

void ParticleSystemManager::AddParticleRendererMachine(RendererMachine* m)
{
	mMachines.Set(m->GetType(), m);
}

StrongRef<RendererMachine> ParticleSystemManager::GetParticleRendererMachine(core::Name n)
{
	auto it = mMachines.Find(n);
	if(it == mMachines.End())
		return nullptr;
	else
		return it.value();
}

core::Range<core::HashMap<core::Name, StrongRef<RendererMachine>>::ConstKeyIterator> ParticleSystemManager::Machines() const
{
	return core::Range<core::HashMap<core::Name, StrongRef<RendererMachine>>::ConstKeyIterator>(mMachines.FirstKey(), mMachines.EndKey());
}

StrongRef<ParticleSystemTemplate> ParticleSystemManager::CreateTemplate()
{
	return LUX_NEW(ParticleSystemTemplate);
}

}
}
