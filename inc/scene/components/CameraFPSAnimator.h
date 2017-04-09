#ifndef INCLUDED_ICAMERAFPSANIMATOR_H
#define INCLUDED_ICAMERAFPSANIMATOR_H
#include "scene/SceneNodeComponent.h"
#include "math/angle.h"

namespace lux
{
namespace scene
{

class CameraFPSAnimator : public SceneNodeComponent
{
public:
	enum EAction
	{
		EA_FORWARD = 0,
		EA_BACKWARD,
		EA_LEFT,
		EA_RIGHT,
		EA_UP,
		EA_DOWN,
		EA_FAST,
		EA_SLOW,

		EA_COUNT
	};
public:
	virtual ~CameraFPSAnimator()
	{}

	virtual void SetKeyCode(EAction action, input::EKeyCode key) = 0;
	virtual input::EKeyCode GetKeyCode(EAction action) const = 0;
	virtual float GetMoveSpeed() const = 0;
	virtual void SetMoveSpeed(float speed) = 0;
	virtual math::anglef GetRotationSpeed() const = 0;
	virtual void SetRotationSpeed(math::anglef speed) = 0;
	virtual bool VerticalMovementAllowed() const = 0;
	virtual void AllowVerticalMovement(bool allow) = 0;
};

}    

}    


#endif