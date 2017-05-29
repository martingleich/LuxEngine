#ifndef INCLUDED_SCENENODEANIMATORROTATION_H
#define INCLUDED_SCENENODEANIMATORROTATION_H
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

class RotationAnimator : public AnimatedSceneNodeComponent
{
public:
	LUX_API RotationAnimator();
	LUX_API RotationAnimator(const math::vector3f& axis, math::anglef RotSpeed);

	void Animate(float time);

	LUX_API void SetAxis(const math::vector3f& axis);
	LUX_API void SetRotationSpeed(math::anglef& speed);

	StrongRef<Referable> Clone() const;
	core::Name GetReferableSubType() const;

private:
	math::vector3f m_Axis;
	math::anglef m_RotSpeed;
};

} // namespace scene
} // namespace lux

#endif