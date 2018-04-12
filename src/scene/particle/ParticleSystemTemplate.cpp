#include "scene/particle/ParticleSystemTemplate.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::ParticleSystemTemplate, "lux.comp.ParticleSystemTemplate");

namespace lux
{
namespace scene
{

namespace
{
template <typename T>
struct ModelCompare
{
	bool Smaller(const T& a, const T& b) const
	{
		return a->GetModel() < b->GetModel();
	}
};
}

ParticleSystemTemplate::ParticleSystemTemplate()
{
}

ParticleSystemTemplate::ParticleSystemTemplate(
	const core::Array<StrongRef<AbstractParticleAffector>>& affectors,
	const core::Array<StrongRef<AbstractParticleEmitter>>& emitters,
	const core::OrderedMap<ParticleModel*, int>& capacities,
	bool isGlobal) :
	m_Affectors(affectors),
	m_Emitters(emitters),
	m_IsGlobal(isGlobal)
{
	// Sort emitters
	m_Affectors.Sort(ModelCompare<AbstractParticleAffector*>());
	m_Emitters.Sort(ModelCompare<AbstractParticleEmitter*>());

	// Generate group data
	// Calculate number of models
	int modelCount = 0;
	ParticleModel* lastModel = nullptr;
	for(auto it = m_Emitters.First(); it != m_Emitters.End(); ++it) {
		if((*it)->GetModel() != lastModel)
			++modelCount;
		lastModel = (*it)->GetModel();
	}
	if(modelCount == 0) {
		m_Affectors.Clear();
		m_Emitters.Clear();
		return;
	}

	// Generate group data for all changed groups
	m_Groups.Resize(modelCount);
	int modId = 0;
	lastModel = m_Emitters.Front()->GetModel();
	float totalFlow = 0.0f;
	for(int i = 0; i < m_Emitters.Size() + 1; ++i) {
		auto model = (i < m_Emitters.Size()) ? m_Emitters[i]->GetModel() : nullptr;
		if(model != lastModel) {
			int capacity = capacities.Get(lastModel, 0);
			auto lifeTime = lastModel->GetLifetime();
			if(capacity == 0 && !lifeTime.IsInfinite())
				capacity = (int)(totalFlow * lifeTime.GetPercentile(0.7f));

			Group group;
			group.model = lastModel;
			group.capacity = capacity;
			group.affectorCount = 0;
			group.firstAffector = 0;
			m_Groups[modId] = group;
			totalFlow = 0.0f;
			++modId;
		} else {
			totalFlow += m_Emitters[i]->GetFlow();
		}
	}

	if(m_Affectors.Size()) {
		lastModel = m_Affectors.Front()->GetModel();
		int count = 0;
		modId = 0;
		int i = 0;
		while(i < m_Affectors.Size() && m_Affectors[i]->GetModel() == nullptr)
			++i;

		for(; i < m_Affectors.Size() + 1; ++i) {
			auto model = (i < m_Affectors.Size()) ? m_Affectors[i]->GetModel() : nullptr;
			if(model != lastModel) {
				m_Groups[modId].firstAffector = i - count;
				m_Groups[modId].affectorCount = count;
				count = 0;
				lastModel = model;
			} else {
				++count;
			}
		}
	}
}

int ParticleSystemTemplate::GetAffectorCount() const
{
	return m_Affectors.Size();
}

StrongRef<AbstractParticleAffector> ParticleSystemTemplate::GetAffector(int i)
{
	return m_Affectors.At(i);
}

StrongRef<AbstractParticleAffector>* ParticleSystemTemplate::GetAffectors()
{
	return m_Affectors.Data();
}

int ParticleSystemTemplate::GetEmitterCount() const
{
	return m_Emitters.Size();
}
StrongRef<AbstractParticleEmitter> ParticleSystemTemplate::GetEmitter(int i)
{
	return m_Emitters.At(i);
}

StrongRef<AbstractParticleEmitter>* ParticleSystemTemplate::GetEmitters()
{
	return m_Emitters.Data();
}

int ParticleSystemTemplate::GetModelCount()
{
	return m_Groups.Size();
}

StrongRef<ParticleModel> ParticleSystemTemplate::GetModel(int modelId)
{
	return m_Groups.At(modelId).model;
}

int ParticleSystemTemplate::GetModelCapacity(int modelId)
{
	return m_Groups.At(modelId).capacity;
}

int ParticleSystemTemplate::GetModelFirstAffector(int modelId)
{
	if(modelId == -1)
		return 0;
	return m_Groups.At(modelId).firstAffector;
}

int ParticleSystemTemplate::GetModelAffectorCount(int modelId)
{
	if(modelId == -1)
		return m_Groups[0].firstAffector;
	return m_Groups.At(modelId).affectorCount;
}

} // namespace scene
} // namespace lux