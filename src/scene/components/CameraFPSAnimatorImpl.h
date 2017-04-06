#ifndef INCLUDED_CAMERAFPSANIMATOR_IMPL_H
#define INCLUDED_CAMERAFPSANIMATOR_IMPL_H
#include "scene/components/CameraFPSAnimator.h"
#include "core/lxArray.h"
#include "math/vector2.h"

namespace lux
{
namespace scene
{

class CameraFPSAnimatorImpl : public CameraFPSAnimator
{
public:
	CameraFPSAnimatorImpl();
	CameraFPSAnimatorImpl(float moveSpeed, math::anglef rotSpeed, math::anglef maxVerticalAngle, bool noVerticalMovement);
	virtual ~CameraFPSAnimatorImpl();
	void Animate(float time);
	void SetActive(bool Active);
	void SetKeyCode(EAction Action, input::EKeyCode key);
	input::EKeyCode GetKeyCode(EAction Action) const;
	StrongRef<Referable> Clone() const;
	bool OnEvent(const input::Event& event);
	bool IsKeyDown(EAction Action) const;
	float GetMoveSpeed() const;
	void SetMoveSpeed(float fSpeed);
	math::anglef GetRotationSpeed() const;
	void SetRotationSpeed(math::anglef fSpeed);
	bool VerticalMovementAllowed() const;
	void AllowVerticalMovement(bool Allow);
	core::Name GetReferableSubType() const;

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

}    // namespace scene
}    // namespace lux

#endif