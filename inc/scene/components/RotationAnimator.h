#ifndef INCLUDED_SCENE_ANIMATOR_ROTATION_H
#define INCLUDED_SCENE_ANIMATOR_ROTATION_H
#include "scene/components/Animator.h"
#include "math/Vector3.h"

namespace lux
{
namespace scene
{

class RotationAnimator : public Animator
{
	LX_REFERABLE_MEMBERS_API(RotationAnimator, LUX_API);
public:
	LUX_API RotationAnimator();
	LUX_API RotationAnimator(const math::Vector3F& axis, math::AngleF RotSpeed);

	LUX_API void Animate(float time);

	LUX_API void SetAxis(const math::Vector3F& axis);
	LUX_API void SetRotationSpeed(const math::AngleF& speed);

	LUX_API const math::Vector3F& GetAxis() const;
	LUX_API const math::AngleF& GetRotationSpeed() const;

private:
	math::Vector3F m_Axis;
	math::AngleF m_RotSpeed;
};

} // namespace scene
} // namespace lux

#endif