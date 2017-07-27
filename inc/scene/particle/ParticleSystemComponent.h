#ifndef INCLUDED_PARTICLESYSTEMNode_H
#define INCLUDED_PARTICLESYSTEMNode_H
#include "scene/components/Animator.h"
#include "scene/Renderable.h"

#include "scene/particle/ParticleGroupData.h"

namespace lux
{
namespace scene
{
class ParticleSystemTemplate;

class ParticleSystem : public Animator, public Renderable
{
public:
	LUX_API ParticleSystem();
	LUX_API ~ParticleSystem();

	LUX_API void AddTemplate(ParticleSystemTemplate* templ);
	LUX_API void RemoveTemplate(ParticleSystemTemplate* templ);
	LUX_API size_t GetTemplateCount() const;
	LUX_API ParticleSystemTemplate* GetTemplate(size_t i);
	LUX_API const ParticleSystemTemplate* GetTemplate(size_t i) const;
	LUX_API void VisitRenderables(RenderableVisitor* visitor, bool noDebug);
	LUX_API void Animate(Node* node, float time);
	LUX_API void Render(Node* node, video::Renderer* renderer, ERenderPass pass);
	LUX_API ERenderPass GetRenderPass() const;
	LUX_API const math::aabbox3df& GetBoundingBox() const;
	LUX_API void UpdateGroups();
	LUX_API StrongRef<ParticleSystemTemplate> ConvertToTemplate();
	LUX_API void SetGlobalParticles(bool isGlobal);
	LUX_API bool IsGlobalParticles() const;
	LUX_API void SetSubtreeScanning(bool scanning);
	LUX_API bool GetSubtreeScanning() const;
	LUX_API core::Name GetReferableType() const;
	LUX_API StrongRef<Referable> Clone() const;

private:
	void CollectGroups(Node* n);
	void CollectGroupsRec(Node* n);
	void UpdateGroupData(Node* n);

private:
	core::Array<StrongRef<ParticleSystemTemplate>> m_Templates;

	core::Array<StrongRef<ParticleGroupData>> m_GroupData;

	core::Array<ParticleGroupData::AffectorEntry> m_GlobalAffectors;
	core::Array<ParticleGroupData::AffectorEntry> m_Affectors; // Sorted by group/model
	core::Array<ParticleGroupData::EmitterEntry> m_Emitters;

	bool m_DirtyGroups = true;
	bool m_UseSubtree = false;
	bool m_IsGlobal = true;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLESYSTEMNode_H
