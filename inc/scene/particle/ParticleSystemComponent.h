#ifndef INCLUDED_LUX_PARTICLESYSTEMNode_H
#define INCLUDED_LUX_PARTICLESYSTEMNode_H
#include "scene/components/Animator.h"

#include "scene/particle/ParticleGroupData.h"

namespace lux
{
namespace scene
{
class ParticleSystemTemplate;

class ParticleSystem : public Component
{
	LX_REFERABLE_MEMBERS_API(ParticleSystem, LUX_API);
public:
	LUX_API ParticleSystem();
	LUX_API ~ParticleSystem();

	LUX_API void SetTemplate(ParticleSystemTemplate* templ);
	LUX_API StrongRef<ParticleSystemTemplate> GetTemplate();

	LUX_API void Animate(float time) override;
	LUX_API void Render(const SceneRenderData& sceneData) override;
	LUX_API RenderPassSet GetRenderPass() const override;
	LUX_API const math::AABBoxF& GetBoundingBox() const override;

private:
	StrongRef<ParticleSystemTemplate> m_Template;
	core::Array<StrongRef<ParticleGroupData>> m_GroupData;
};

} // namespace scene
} // namespace lux

#endif // !INCLUDED_LUX_PARTICLESYSTEMNode_H
