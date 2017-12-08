#include "scene/components/FirstPersonCameraControl.h"
#include "scene/Node.h"
#include "core/Logger.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::FirstPersonCameraControl, lux::scene::SceneComponentType::FirstPersonCameraControl);

namespace lux
{
namespace scene
{

FirstPersonCameraControl::FirstPersonCameraControl() :
	FirstPersonCameraControl(4.0f, math::AngleF::Degree(30.0f), math::AngleF::Degree(89.0f), false)
{
}

FirstPersonCameraControl::FirstPersonCameraControl(float moveSpeed, math::AngleF rotSpeed, math::AngleF maxAngle, bool noVertical) :
	m_MoveSpeed(moveSpeed),
	m_RotSpeed(rotSpeed),
	m_MaxVerticalAngle(maxAngle),
	m_NoVerticalMovement(noVertical),
	m_Fast(false)
{
	EnableInput();
}

FirstPersonCameraControl::~FirstPersonCameraControl()
{
}

void FirstPersonCameraControl::Animate(float time)
{
	auto node = GetParent();
	if(!node)
		return;

	math::Vector3F pos = node->GetAbsolutePosition();
	math::Vector3F flank = node->FromRelativeDir(math::Vector3F::UNIT_X);
	math::Vector3F up = node->FromRelativeDir(math::Vector3F::UNIT_Y);
	math::Vector3F look = node->FromRelativeDir(math::Vector3F::UNIT_Z);

	math::Vector3F move = look;
	if(m_NoVerticalMovement) {
		move.y = 0.0f;
		move.Normalize();
	}

	auto deltaRotY = m_Rot.y * m_RotSpeed;
	auto deltaRotX = -m_Rot.x * m_RotSpeed;

	auto curAngleX = math::ArcCos(up.y);
	if(look.y < 0)
		curAngleX = -curAngleX;

	if(curAngleX + deltaRotX > m_MaxVerticalAngle)
		deltaRotX = m_MaxVerticalAngle - curAngleX;
	else if(curAngleX + deltaRotX < -m_MaxVerticalAngle)
		deltaRotX = -m_MaxVerticalAngle - curAngleX;

	auto rot =
		math::QuaternionF(flank, -deltaRotX) *
		math::QuaternionF(math::Vector3F::UNIT_Y, deltaRotY);

	node->SetOrientation(node->GetOrientation() * rot);

	auto speed = m_MoveSpeed * (m_Fast ? 2.0f : 1.0f);
	pos += move * m_Move.z * speed * time;
	pos += flank * m_Move.x * speed * time;

	if(!m_NoVerticalMovement)
		pos.y += m_Move.y * speed * time;

	node->SetPosition(pos);

	m_Rot.Set(0, 0);
}

float FirstPersonCameraControl::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

void FirstPersonCameraControl::SetMoveSpeed(float fSpeed)
{
	m_MoveSpeed = fSpeed;
}

void FirstPersonCameraControl::SetMaxVerticalAngle(math::AngleF a)
{
	m_MaxVerticalAngle = a;
}

math::AngleF FirstPersonCameraControl::GetMaxVerticalAngle() const
{
	return m_MaxVerticalAngle;
}

math::AngleF FirstPersonCameraControl::GetRotationSpeed() const
{
	return m_RotSpeed;
}

void FirstPersonCameraControl::SetRotationSpeed(math::AngleF fSpeed)
{
	m_RotSpeed = fSpeed;
}

bool FirstPersonCameraControl::VerticalMovementAllowed() const
{
	return !m_NoVerticalMovement;
}

void FirstPersonCameraControl::AllowVerticalMovement(bool Allow)
{
	m_NoVerticalMovement = !Allow;
}

void FirstPersonCameraControl::RotX(float x)
{
	m_Rot.x += x;
}

void FirstPersonCameraControl::RotY(float y)
{
	m_Rot.y += y;
}

void FirstPersonCameraControl::ForwardSpeed(float f)
{
	m_Move.z = f;
}

void FirstPersonCameraControl::FlankSpeed(float f)
{
	m_Move.x = f;
}

void FirstPersonCameraControl::UpSpeed(float f)
{
	m_Move.y = f;
}

void FirstPersonCameraControl::SetFast(bool fast)
{
	m_Fast = fast;
}

void FirstPersonCameraControl::EnableInput()
{
	ForwardSpeed(m_KeyboardController.m_Move.z);
	FlankSpeed(m_KeyboardController.m_Move.x);
	UpSpeed(m_KeyboardController.m_Move.y);
	m_IsControlActive = true;
}

void FirstPersonCameraControl::DisableInput()
{
	ForwardSpeed(0);
	FlankSpeed(0);
	UpSpeed(0);
	m_IsControlActive = false;
}

bool FirstPersonCameraControl::IsInputActive()
{
	return m_IsControlActive;
}

void FirstPersonCameraControl::DefaultEventHandler(const input::Event& event)
{
	if(event.type == input::EEventType::Button && event.source == input::EEventSource::Keyboard) {
		float value = event.button.state ? 1.0f : -1.0f;
		bool changed = true;
		switch(event.button.code) {
		case input::KEY_KEY_W: m_KeyboardController.m_Move.z += value; break;
		case input::KEY_KEY_S: m_KeyboardController.m_Move.z -= value; break;
		case input::KEY_KEY_D: m_KeyboardController.m_Move.x += value; break;
		case input::KEY_KEY_A: m_KeyboardController.m_Move.x -= value; break;
		case input::KEY_KEY_Q: m_KeyboardController.m_Move.y += value; break;
		case input::KEY_KEY_E: m_KeyboardController.m_Move.y -= value; break;
		case input::KEY_LSHIFT: SetFast(event.button.state); break;
		default: changed = false;
		}
		if(changed && m_IsControlActive) {
			ForwardSpeed(m_KeyboardController.m_Move.z);
			FlankSpeed(m_KeyboardController.m_Move.x);
			UpSpeed(m_KeyboardController.m_Move.y);
		}
	} else if(event.type == input::EEventType::Area && event.source == input::EEventSource::Mouse) {
		if(m_IsControlActive) {
			RotX(event.area.relY);
			RotY(event.area.relX);
		}
	}
}

}
}
