#ifndef INCLUDED_LUX_FIRST_PERSON_CAMERA_CONTROL_H
#define INCLUDED_LUX_FIRST_PERSON_CAMERA_CONTROL_H
#include "scene/components/Animator.h"
#include "math/Angle.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "input/InputEvent.h"

namespace lux
{
namespace scene
{

class FirstPersonCameraControl : public Animator
{
	LX_REFERABLE_MEMBERS_API(FirstPersonCameraControl, LUX_API);
public:
	struct KeyboardMouseControls
	{
		input::EKeyCode forward;
		input::EKeyCode backward;
		input::EKeyCode left;
		input::EKeyCode right;
		input::EKeyCode up;
		input::EKeyCode down;
		input::EKeyCode fast;

		bool invertX;
		bool invertY;
	};

	LUX_API FirstPersonCameraControl();
	LUX_API FirstPersonCameraControl(float moveSpeed, math::AngleF rotSpeed, math::AngleF maxVerticalAngle, bool noVerticalMovement);

	LUX_API ~FirstPersonCameraControl();

	LUX_API void Animate(float time);

	LUX_API float GetMoveSpeed() const;
	LUX_API void SetMoveSpeed(float fSpeed);

	LUX_API void SetMaxVerticalAngle(math::AngleF a);
	LUX_API math::AngleF GetMaxVerticalAngle() const;

	LUX_API math::AngleF GetRotationSpeed() const;
	LUX_API void SetRotationSpeed(math::AngleF fSpeed);

	LUX_API bool VerticalMovementAllowed() const;
	LUX_API void AllowVerticalMovement(bool Allow);

	LUX_API void RotX(float x);
	LUX_API void RotY(float y);
	LUX_API void ForwardSpeed(float f);
	LUX_API void FlankSpeed(float f);
	LUX_API void UpSpeed(float f);
	LUX_API void SetFast(bool fast);

	// Controller functions
	LUX_API void HandleInput(const input::Event& event);
	LUX_API void EnableInput();
	LUX_API void DisableInput();
	LUX_API bool IsInputActive();

	LUX_API KeyboardMouseControls GetKeyboardMouseControls() const;
	LUX_API void SetKeyboardMouseControls(const KeyboardMouseControls& controls);

private:
	float m_MoveSpeed;
	math::AngleF m_RotSpeed;
	math::AngleF m_MaxVerticalAngle;

	math::Vector2F m_Rot;
	math::Vector3F m_Move;

	bool m_NoVerticalMovement;
	bool m_Fast;

	bool m_IsControlActive;

	KeyboardMouseControls m_Controls;
	math::Vector3F m_KeyboardMouseMove;
};

} // namespace scene
} // namespace lux

#endif
