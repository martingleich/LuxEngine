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
	SetAnimated(true);
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::SetTemplate(ParticleSystemTemplate* templ)
{
	m_Template = templ;

	m_GroupData.Clear();

	auto modelCount = m_Template->GetModelCount();
	for(int i = 0; i < modelCount; ++i) {
		auto model = m_Template->GetModel(i);
		auto capacity = m_Template->GetModelCapacity(i);

		m_GroupData.PushBack(LUX_NEW(ParticleGroupData)(model, capacity));
	}
}

StrongRef<ParticleSystemTemplate> ParticleSystem::GetTemplate()
{
	return m_Template;
}

void ParticleSystem::Animate(float time)
{
	auto node = GetNode();
	if(!node)
		return;

	ParticleGroupData::SystemData data;
	data.psSystem = node;

	data.emitters = m_Template->GetEmitters();
	data.emitterCount = m_Template->GetEmitterCount();

	data.affectors[0] = m_Template->GetAffectors() + m_Template->GetModelFirstAffector(-1);
	data.affectorsCounts[0] = m_Template->GetModelAffectorCount(-1);

	for(int i = 0; i < m_GroupData.Size(); ++i) {
		data.affectors[1] = m_Template->GetAffectors() + m_Template->GetModelFirstAffector(i);
		data.affectorsCounts[1] = m_Template->GetModelAffectorCount(i);
		m_GroupData[i]->Update(time, data);
	}
}

void ParticleSystem::Render(const SceneRenderData& r)
{
	if(r.pass != ERenderPass::Transparent)
		return;
	auto node = GetNode();
	if(!node)
		return;

	if(m_Template->IsGlobal()) {
		r.video->SetTransform(video::ETransform::World, math::Matrix4::IDENTITY);
	} else {
		math::Matrix4 mat;
		node->GetAbsoluteTransform().ToMatrix(mat);
		r.video->SetTransform(video::ETransform::World, mat);
	}

	for(auto& g : m_GroupData) {
		auto pr = g->GetModel()->GetRenderer();
		pr->Render(r.video, g);
	}
}

RenderPassSet ParticleSystem::GetRenderPass() const
{
	return RenderPassSet(ERenderPass::Transparent);
}

const math::AABBoxF& ParticleSystem::GetBoundingBox() const
{
	return math::AABBoxF::EMPTY;
}

}
}
