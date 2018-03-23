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
	LX_REFERABLE_MEMBERS_API(ParticleSystem, LUX_API);
public:
	LUX_API ParticleSystem();
	LUX_API ~ParticleSystem();

	LUX_API void SetTemplate(ParticleSystemTemplate* templ);
	LUX_API StrongRef<ParticleSystemTemplate> GetTemplate();

	LUX_API void VisitRenderables(RenderableVisitor* visitor, scene::ERenderableTags tags);
	LUX_API void Animate(float time);
	LUX_API void Render(Node* node, video::Renderer* renderer, const SceneData& sceneData);
	LUX_API ERenderPass GetRenderPass() const;
	LUX_API const math::AABBoxF& GetBoundingBox() const;

private:
	StrongRef<ParticleSystemTemplate> m_Template;
	core::Array<StrongRef<ParticleGroupData>> m_GroupData;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_PARTICLESYSTEMNode_H
