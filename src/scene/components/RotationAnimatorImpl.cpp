#include "RotationAnimatorImpl.h"

namespace lux
{
namespace scene
{

SceneNodeAnimatorRotationImpl::SceneNodeAnimatorRotationImpl()
	: m_RotSpeed(0.0f)
{
}

SceneNodeAnimatorRotationImpl::SceneNodeAnimatorRotationImpl(const math::vector3f& axis, math::anglef RotSpeed)
	: m_Axis(axis), m_RotSpeed(RotSpeed)
{
}

void SceneNodeAnimatorRotationImpl::Animate(float time)
{
	SceneNode* node = GetParent();
	if(!node)
		return;

	math::quaternionf quat = node->GetOrientation();
	quat *= math::quaternionf(m_Axis, time*m_RotSpeed);
	quat.Normalize();
	node->SetOrientation(quat);
}

StrongRef<Referable> SceneNodeAnimatorRotationImpl::Clone() const
{
	return LUX_NEW(SceneNodeAnimatorRotationImpl)(m_Axis, m_RotSpeed);
}

core::Name SceneNodeAnimatorRotationImpl::GetReferableSubType() const
{
	return SceneNodeComponentType::Rotation;
}

}    // namespace scene
}    // namespace lux