#ifndef INCLUDED_FPS_CONTROL_H
#define INCLUDED_FPS_CONTROL_H
#include "scene/components/Animator.h"
#include "math/angle.h"
#include "math/vector2.h"

#include "events/lxActions.h"

namespace lux
{
namespace scene
{

class CameraControl : public Animator
{
public:
	LUX_API CameraControl();
	LUX_API CameraControl(float moveSpeed, math::anglef rotSpeed, math::anglef maxVerticalAngle, bool noVerticalMovement);

	LUX_API ~CameraControl();

	LUX_API void Animate(Node* node, float time);

	LUX_API float GetMoveSpeed() const;
	LUX_API void SetMoveSpeed(float fSpeed);

	LUX_API void SetMaxVerticalAngle(math::anglef a);
	LUX_API math::anglef GetMaxVerticalAngle() const;

	LUX_API math::anglef GetRotationSpeed() const;
	LUX_API void SetRotationSpeed(math::anglef fSpeed);

	LUX_API bool VerticalMovementAllowed() const;
	LUX_API void AllowVerticalMovement(bool Allow);

	LUX_API core::Name GetReferableSubType() const;
	LUX_API StrongRef<Referable> Clone() const;

	//! Maps a event to the camera actions
	/**
	This mapping used WASD to move the camera, QE to move the camera up and down,
	and the mouse to look around.
	*/
	LUX_API static void DefaultEventToCameraAction(const input::Event& event);

private:
	void MouseMoveX(float v);
	void MouseMoveY(float v);

private:
	float m_MoveSpeed;
	math::anglef m_RotSpeed;
	math::anglef m_MaxVerticalAngle;
	bool m_NoVerticalMovement;

	WeakRef<events::AxisAction> m_Forward;
	WeakRef<events::AxisAction> m_Flank;
	WeakRef<events::AxisAction> m_Up;

	math::vector2f m_MouseMove;
};

} // namespace scene
} // namespace lux

#endif