#include "scene/components/RotationAnimator.h"
#include "scene/Node.h"
#include "math/quaternion.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.comp.Rotation", lux::scene::RotationAnimator)

namespace lux
{
namespace scene
{

RotationAnimator::RotationAnimator()
	: m_RotSpeed(0.0f)
{
}

RotationAnimator::RotationAnimator(const math::vector3f& axis, math::anglef RotSpeed)
	: m_Axis(axis), m_RotSpeed(RotSpeed)
{
}

void RotationAnimator::Animate(Node* node, float time)
{
	if(!node)
		return;

	math::quaternionf quat = node->GetOrientation();
	quat *= math::quaternionf(m_Axis, time*m_RotSpeed);
	quat.Normalize();
	node->SetOrientation(quat);
}

void RotationAnimator::SetAxis(const math::vector3f& axis)
{
	m_Axis = axis;
}

void RotationAnimator::SetRotationSpeed(const math::anglef& speed)
{
	m_RotSpeed = speed;
}

const math::vector3f& RotationAnimator::GetAxis() const
{
	return m_Axis;
}

const math::anglef& RotationAnimator::GetRotationSpeed() const
{
	return m_RotSpeed;
}

StrongRef<Referable> RotationAnimator::Clone() const
{
	return LUX_NEW(RotationAnimator)(m_Axis, m_RotSpeed);
}

core::Name RotationAnimator::GetReferableType() const
{
	return SceneComponentType::Rotation;
}

} // namespace scene
} // namespace lux
