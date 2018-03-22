#include "scene/particle/ParticleSystemComponent.h"
#include "scene/particle/ParticleSystemTemplate.h"

#include "scene/Node.h"
#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::ParticleSystem, "lux.comp.ParticleSystem");

namespace lux
{
namespace scene
{

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::AddTemplate(ParticleSystemTemplate* templ)
{
	m_Templates.PushBack(templ);
}

void ParticleSystem::RemoveTemplate(ParticleSystemTemplate* templ)
{
	for(auto it = m_Templates.First(); it != m_Templates.End(); ++it) {
		if(*it == templ) {
			m_Templates.Erase(it);
			return;
		}
	}
}

int ParticleSystem::GetTemplateCount() const
{
	return m_Templates.Size();
}

ParticleSystemTemplate* ParticleSystem::GetTemplate(int i)
{
	return m_Templates.At(i);
}

const ParticleSystemTemplate* ParticleSystem::GetTemplate(int i) const
{
	return m_Templates.At(i);
}

void ParticleSystem::VisitRenderables(RenderableVisitor* visitor, scene::ERenderableTags tags)
{
	LUX_UNUSED(tags);

	visitor->Visit(GetParent(), this);
}

void ParticleSystem::Animate(float time)
{
	auto node = GetParent();
	if(!node)
		return;
	UpdateGroupData(node);

	ParticleGroupData::SystemData data;
	data.emitters = m_Emitters.Data();
	data.emitterCount = m_Emitters.Size();
	data.globals = m_GlobalAffectors.Data();
	data.globalCount = m_GlobalAffectors.Size();

	data.owner = node;
	data.psSystem = m_IsGlobal ? node->GetRoot() : node;

	const math::Transformation& psTrans = data.psSystem->GetAbsoluteTransform();
	math::Transformation psTransI = psTrans.GetInverted();

	for(auto it = m_Emitters.First(); it != m_Emitters.End(); ++it) {
		const math::Transformation& ownerTrans = it->n ?
			it->n->GetAbsoluteTransform() :
			data.owner->GetAbsoluteTransform();
		if(it->e->IsGlobal())
			it->transform = psTransI;
		else
			it->transform = ownerTrans.CombineLeft(psTransI);
	}

	int ai = 0;
	for(int i = 0; i < m_GroupData.Size(); ++i) {
		int ac = 0;
		for(; ai < m_Affectors.Size(); ++ai) {
			if(m_Affectors[ai]->GetModel() != m_GroupData[i]->GetModel())
				break;
			++ac;
		}

		data.locals = m_Affectors.Data() + ai;
		data.localCount = ac;

		m_GroupData[i]->Update(time, data);
	}

	for(auto it = m_Emitters.First(); it != m_Emitters.End(); ++it)
		it->lastTransform = it->transform;
}

void ParticleSystem::Render(Node* node, video::Renderer* renderer, const SceneData& sceneData)
{
	UpdateGroupData(node);

	if(sceneData.pass != ERenderPass::Transparent)
		return;

	if(m_IsGlobal) {
		renderer->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);
	} else {
		math::Matrix4 mat;
		node->GetAbsoluteTransform().ToMatrix(mat);
		renderer->SetTransform(video::ETransform::World, mat);
	}

	for(auto& g : m_GroupData) {
		auto r = g->GetModel()->GetRenderer();
		r->GetMachine()->Render(renderer, g, r);
	}
}

ERenderPass ParticleSystem::GetRenderPass() const
{
	return ERenderPass::Transparent;
}

const math::AABBoxF& ParticleSystem::GetBoundingBox() const
{
	return math::AABBoxF::EMPTY;
}

void ParticleSystem::UpdateGroups()
{
	m_DirtyGroups = true;
}

StrongRef<ParticleSystemTemplate> ParticleSystem::ConvertToTemplate()
{
	auto templ = LUX_NEW(ParticleSystemTemplate);

	// TODO: Bake transforms into emitters
	for(auto it = m_Affectors.First(); it != m_Affectors.End(); ++it)
		templ->AddAffector(it->a);

	for(auto it = m_Emitters.First(); it != m_Emitters.End(); ++it)
		templ->AddEmitter(it->e);

	templ->Finalize();

	return templ;
}

void ParticleSystem::SetGlobalParticles(bool isGlobal)
{
	m_IsGlobal = isGlobal;
}

bool ParticleSystem::IsGlobalParticles() const
{
	return m_IsGlobal;
}

void ParticleSystem::SetSubtreeScanning(bool scanning)
{
	m_UseSubtree = scanning;
}

bool ParticleSystem::GetSubtreeScanning() const
{
	return m_UseSubtree;
}

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

void ParticleSystem::CollectGroups(Node* n)
{
	m_GlobalAffectors.Clear();
	m_Affectors.Clear();
	m_Emitters.Clear();

	for(auto tmpl : m_Templates) {
		for(int i = 0; i < tmpl->GetAffectorCount(); ++i) {
			auto aff = tmpl->GetAffector(i);
			if(aff->GetModel())
				m_Affectors.PushBack(ParticleGroupData::AffectorEntry(aff));
			else
				m_GlobalAffectors.PushBack(ParticleGroupData::AffectorEntry(aff));
		}
		for(int i = 0; i < tmpl->GetEmitterCount(); ++i) {
			auto em = tmpl->GetEmitter(i);
			if(em->GetModel())
				m_Emitters.PushBack(ParticleGroupData::EmitterEntry(em));
		}
	}

	if(m_UseSubtree)
		CollectGroupsRec(n);

	m_Affectors.Sort(ModelCompare<ParticleGroupData::AffectorEntry>());
	m_Emitters.Sort(ModelCompare<ParticleGroupData::EmitterEntry>());
}

void ParticleSystem::CollectGroupsRec(Node* n)
{
	for(auto comp : n->Components()) {
		auto aff = dynamic_cast<ParticleAffector*>(comp);
		auto em = dynamic_cast<ParticleEmitter*>(comp);
		if(aff) {
			if(aff->GetModel())
				m_Affectors.PushBack(ParticleGroupData::AffectorEntry(aff, n));
			else
				m_GlobalAffectors.PushBack(ParticleGroupData::AffectorEntry(aff, n));
		}
		if(em) {
			if(em->GetModel())
				m_Emitters.PushBack(ParticleGroupData::EmitterEntry(em, n));
		}
	}

	for(auto child : n->Children())
		CollectGroupsRec(child);
}

void ParticleSystem::UpdateGroupData(Node* n)
{
	if(!m_DirtyGroups)
		return;

	CollectGroups(n);

	// Calculate number of models
	u32 modelCount = 0;
	ParticleModel* lastModel = nullptr;
	for(auto it = m_Emitters.First(); it != m_Emitters.End(); ++it) {
		if((*it)->GetModel() != lastModel)
			++modelCount;
		lastModel = (*it)->GetModel();
	}

	// Generate group data for all changed groups
	m_GroupData.Resize(modelCount);
	int modId = 0;
	lastModel = m_Emitters.Front()->GetModel();
	float totalFlow = 0.0f;
	for(int i = 0; i < m_Emitters.Size() + 1; ++i) {
		auto model = (i < m_Emitters.Size()) ? m_Emitters[i]->GetModel() : nullptr;
		if(model != lastModel) {
			u32 capacity;
			if(lastModel->GetCapacity())
				capacity = lastModel->GetCapacity();
			else {
				float min, max;
				lastModel->GetLifeTime(min, max);
				capacity = (u32)(totalFlow * max);
			}
			m_GroupData[modId] = LUX_NEW(ParticleGroupData)(lastModel, capacity);
			totalFlow = 0.0f;
			++modId;
		} else {
			totalFlow += m_Emitters[i]->GetFlow();
		}
	}

	m_DirtyGroups = false;
}
}
}
