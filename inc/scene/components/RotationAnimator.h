#ifndef INCLUDED_SCENE_ANIMATOR_ROTATION_H
#define INCLUDED_SCENE_ANIMATOR_ROTATION_H
#include "scene/components/Animator.h"
#include "math/vector3.h"

namespace lux
{
namespace scene
{

class RotationAnimator : public Animator
{
public:
	LUX_API RotationAnimator();
	LUX_API RotationAnimator(const math::vector3f& axis, math::anglef RotSpeed);

	LUX_API void Animate(Node* node, float time);

	LUX_API void SetAxis(const math::vector3f& axis);
	LUX_API void SetRotationSpeed(const math::anglef& speed);

	LUX_API const math::vector3f& GetAxis() const;
	LUX_API const math::anglef& GetRotationSpeed() const;

	LUX_API StrongRef<Referable> Clone() const;
	LUX_API core::Name GetReferableType() const;

private:
	math::vector3f m_Axis;
	math::anglef m_RotSpeed;
};

} // namespace scene
} // namespace lux

#endif