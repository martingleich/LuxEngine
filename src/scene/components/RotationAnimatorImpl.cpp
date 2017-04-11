#include "RotationAnimatorImpl.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::SceneNodeAnimatorRotationImpl)

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

void SceneNodeAnimatorRotationImpl::SetAxis(const math::vector3f& axis)
{
	m_Axis = axis;
}

void SceneNodeAnimatorRotationImpl::SetRotationSpeed(math::anglef& speed)
{
	m_RotSpeed = speed;
}

StrongRef<Referable> SceneNodeAnimatorRotationImpl::Clone() const
{
	return LUX_NEW(SceneNodeAnimatorRotationImpl)(m_Axis, m_RotSpeed);
}

core::Name SceneNodeAnimatorRotationImpl::GetReferableSubType() const
{
	return SceneNodeComponentType::Rotation;
}

}    

}    

