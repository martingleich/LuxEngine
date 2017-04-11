#ifndef INCLUDED_SCENENODEANIMATORROTATION_IMPL_H
#define INCLUDED_SCENENODEANIMATORROTATION_IMPL_H
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

class SceneNodeAnimatorRotationImpl : public AnimatedSceneNodeComponent
{
public:
	SceneNodeAnimatorRotationImpl();
	SceneNodeAnimatorRotationImpl(const math::vector3f& axis, math::anglef RotSpeed);

	virtual void Animate(float time);
	virtual StrongRef<Referable> Clone() const;
	virtual core::Name GetReferableSubType() const;

	void SetAxis(const math::vector3f& axis);
	void SetRotationSpeed(math::anglef& speed);

private:
	math::vector3f m_Axis;
	math::anglef m_RotSpeed;
};

}    

}    


#endif