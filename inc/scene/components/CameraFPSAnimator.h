#ifndef INCLUDED_CAMERAFPSANIMATOR_H
#define INCLUDED_CAMERAFPSANIMATOR_H
#include "scene/SceneNodeComponent.h"
#include "math/angle.h"
#include "math/vector2.h"

namespace lux
{
namespace scene
{

class CameraFPSAnimator : public AnimatedSceneNodeComponent
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
	LUX_API CameraFPSAnimator();
	LUX_API CameraFPSAnimator(float moveSpeed, math::anglef rotSpeed, math::anglef maxVerticalAngle, bool noVerticalMovement);

	void Animate(float time);
	void SetActive(bool Active);

	LUX_API void SetKeyCode(EAction Action, input::EKeyCode key);
	LUX_API input::EKeyCode GetKeyCode(EAction Action) const;
	LUX_API bool OnEvent(const input::Event& event);
	LUX_API bool IsKeyDown(EAction Action) const;
	LUX_API float GetMoveSpeed() const;
	LUX_API void SetMoveSpeed(float fSpeed);
	LUX_API void SetMaxVerticalAngle(math::anglef a) { m_MaxVerticalAngle = a; }
	LUX_API math::anglef GetRotationSpeed() const;
	LUX_API void SetRotationSpeed(math::anglef fSpeed);
	LUX_API bool VerticalMovementAllowed() const;
	LUX_API void AllowVerticalMovement(bool Allow);

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	struct SKey
	{
		input::EKeyCode keyCode;
		bool state;

		SKey() : keyCode(input::KEY_NONE), state(false)
		{}
		SKey(input::EKeyCode key) : keyCode(key), state(false)
		{}
	};

private:
	float m_MoveSpeed;
	math::anglef m_RotSpeed;
	math::anglef m_MaxVerticalAngle;

	SKey m_KeyMap[EA_COUNT];
	bool m_NoVerticalMovement;
	bool m_FirstCall;

	math::vector2f m_MouseMove;
};

}    

}    


#endif