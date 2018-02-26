#include "scene/components/FirstPersonCameraControl.h"
#include "scene/Node.h"
#include "core/Logger.h"
#include "input/InputSystem.h"

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
	m_Controls.forward = input::EKeyCode::KEY_KEY_W;
	m_Controls.backward = input::EKeyCode::KEY_KEY_S;
	m_Controls.left = input::EKeyCode::KEY_KEY_A;
	m_Controls.right = input::EKeyCode::KEY_KEY_D;
	m_Controls.up = input::EKeyCode::KEY_KEY_Q;
	m_Controls.down = input::EKeyCode::KEY_KEY_E;
	m_Controls.fast = input::EKeyCode::KEY_LSHIFT;
	m_Controls.invertX = false;
	m_Controls.invertY = false;

	input::InputSystem::Instance()->GetEventSignal().Connect(this, &FirstPersonCameraControl::HandleInput);
}

FirstPersonCameraControl::~FirstPersonCameraControl()
{
}

void FirstPersonCameraControl::Animate(float time)
{
	auto node = GetParent();
	if(!node)
		return;
	math::Vector3F pos;
	math::Vector3F flank;
	math::Vector3F up;
	math::Vector3F look;
	pos = node->GetPosition();
	flank = node->FromRelativeDir(math::Vector3F::UNIT_X, node->GetParent());
	up = node->FromRelativeDir(math::Vector3F::UNIT_Y, node->GetParent());
	look = node->FromRelativeDir(math::Vector3F::UNIT_Z, node->GetParent());

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
	ForwardSpeed(m_KeyboardMouseMove.z);
	FlankSpeed(m_KeyboardMouseMove.x);
	UpSpeed(m_KeyboardMouseMove.y);
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

void FirstPersonCameraControl::HandleInput(const input::Event& event)
{
	if(event.type == input::EEventType::Button && event.source == input::EEventSource::Keyboard) {
		float value = event.button.state ? 1.0f : -1.0f;
		bool changed = true;
		if(event.button.code == m_Controls.forward)
			m_KeyboardMouseMove.z += value;
		else if(event.button.code == m_Controls.backward)
			m_KeyboardMouseMove.z -= value;
		else if(event.button.code == m_Controls.left)
			m_KeyboardMouseMove.x -= value;
		else if(event.button.code == m_Controls.right)
			m_KeyboardMouseMove.x += value;
		else if(event.button.code == m_Controls.up)
			m_KeyboardMouseMove.y += value;
		else if(event.button.code == m_Controls.down)
			m_KeyboardMouseMove.y -= value;
		else if(event.button.code == m_Controls.fast)
			SetFast(event.button.state);
		else
			changed = false;
		if(changed && m_IsControlActive) {
			ForwardSpeed(m_KeyboardMouseMove.z);
			FlankSpeed(m_KeyboardMouseMove.x);
			UpSpeed(m_KeyboardMouseMove.y);
		}
	} else if(event.type == input::EEventType::Area && event.source == input::EEventSource::Mouse) {
		if(m_IsControlActive) {
			RotX((m_Controls.invertY ? -1 : 1) * event.area.relY);
			RotY((m_Controls.invertX ? -1 : 1) * event.area.relX);
		}
	}
}

FirstPersonCameraControl::KeyboardMouseControls FirstPersonCameraControl::GetKeyboardMouseControls() const
{
	return m_Controls;
}

void FirstPersonCameraControl::SetKeyboardMouseControls(const KeyboardMouseControls& controls)
{
	m_Controls = controls;
}

} // namespace scene
} // namespace lux
