#ifndef INCLUDED_FIRST_PERSON_CAMERA_CONTROL_H
#define INCLUDED_FIRST_PERSON_CAMERA_CONTROL_H
#include "scene/components/Animator.h"
#include "math/Angle.h"
#include "math/vector2.h"
#include "math/vector3.h"

#include "input/InputEvent.h"

namespace lux
{
namespace scene
{

class FirstPersonCameraControl : public Animator
{
public:
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

	LUX_API core::Name GetReferableType() const;
	LUX_API StrongRef<Referable> Clone() const;

	LUX_API void RotX(float x);
	LUX_API void RotY(float y);
	LUX_API void ForwardSpeed(float f);
	LUX_API void FlankSpeed(float f);
	LUX_API void UpSpeed(float f);
	LUX_API void SetFast(bool fast);

	//! Maps a event to the camera actions
	/**
	This mapping used WASD to move the camera, QE to move the camera up and down,
	and the mouse to look around.
	*/
	LUX_API void DefaultEventToCameraAction(const input::Event& event);

private:
	float m_MoveSpeed;
	math::AngleF m_RotSpeed;
	math::AngleF m_MaxVerticalAngle;
	bool m_NoVerticalMovement;

	bool m_Fast;
	math::Vector2F m_Rot;
	math::Vector3F m_Move;
};

} // namespace scene
} // namespace lux

#endif
