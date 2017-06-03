#include "scene/components/CameraControl.h"
#include "scene/Node.h"
#include "core/ReferableRegister.h"
#include "core/Logger.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::CameraControl)

namespace lux
{
namespace scene
{

CameraControl::CameraControl() :
	CameraControl(4.0f, math::anglef::Degree(90.0f), math::anglef::Degree(89.0f), false)
{
}

CameraControl::CameraControl(float moveSpeed, math::anglef rotSpeed, math::anglef maxAngle, bool noVertical) : 
	m_MoveSpeed(moveSpeed),
	m_RotSpeed(rotSpeed),
	m_MaxVerticalAngle(maxAngle),
	m_NoVerticalMovement(noVertical),
	m_MouseMove(0.0f, 0.0f)
{
	m_IsEventReceiver = true;

	m_KeyMap[EA_FORWARD] = input::KEY_KEY_W;
	m_KeyMap[EA_BACKWARD] = input::KEY_KEY_S;
	m_KeyMap[EA_LEFT] = input::KEY_KEY_A;
	m_KeyMap[EA_RIGHT] = input::KEY_KEY_D;
	m_KeyMap[EA_FAST] = input::KEY_LSHIFT;
	m_KeyMap[EA_SLOW] = input::KEY_LCONTROL;
	m_KeyMap[EA_UP] = input::KEY_KEY_Q;
	m_KeyMap[EA_DOWN] = input::KEY_KEY_E;
}

void CameraControl::Animate(Node* node, float time)
{
	node->UpdateAbsTransform();

	math::vector3f pos = node->GetAbsoluteTransform().translation;
	math::vector3f flank = node->FromRelativeDir(math::vector3f::UNIT_X);
	math::vector3f up = node->FromRelativeDir(math::vector3f::UNIT_Y);
	math::vector3f look = node->FromRelativeDir(math::vector3f::UNIT_Z);

	math::vector3f move = look;
	if(m_NoVerticalMovement) {
		move.y = 0.0f;
		move.Normalize();
	}

	auto deltaRotY = m_MouseMove.x * m_RotSpeed * 0.001f;
	auto deltaRotX = -m_MouseMove.y * m_RotSpeed * 0.001f;

	auto curAngleX = math::ArcCos(up.y);
	if(look.y < 0)
		curAngleX = -curAngleX;

	if(curAngleX + deltaRotX > m_MaxVerticalAngle)
		deltaRotX = m_MaxVerticalAngle - curAngleX;
	else if(curAngleX + deltaRotX < -m_MaxVerticalAngle)
		deltaRotX = -m_MaxVerticalAngle - curAngleX;

	auto rot =
		math::quaternionf(flank, -deltaRotX) *
		math::quaternionf(math::vector3f::UNIT_Y, deltaRotY);

	node->SetOrientation(node->GetOrientation() * rot);

	float realSpeed = m_MoveSpeed;
	if(IsKeyDown(EA_FAST))
		realSpeed *= 2.0f;

	if(IsKeyDown(EA_SLOW))
		realSpeed *= 0.5f;

	if(IsKeyDown(EA_FORWARD))
		pos += move * realSpeed * time;

	if(IsKeyDown(EA_BACKWARD))
		pos -= move * realSpeed * time;

	if(IsKeyDown(EA_LEFT))
		pos -= flank * realSpeed * time;

	if(IsKeyDown(EA_RIGHT))
		pos += flank * realSpeed * time;

	if(m_NoVerticalMovement == false) {
		if(IsKeyDown(EA_UP))
			pos.y += realSpeed * time;

		if(IsKeyDown(EA_DOWN))
			pos.y -= realSpeed * time;
	}

	node->SetPosition(pos);

	m_MouseMove.Set(0.0f, 0.0f);
}

StrongRef<Referable> CameraControl::Clone() const
{
	StrongRef<CameraControl> out = LUX_NEW(CameraControl)(m_MoveSpeed, m_RotSpeed,
		m_MaxVerticalAngle, m_NoVerticalMovement);

	for(int i = 0; i < EA_COUNT; ++i)
		out->SetKeyCode((EAction)i, this->GetKeyCode((EAction)i));

	return out;
}

void CameraControl::SetKeyCode(EAction Action, input::EKeyCode key)
{
	m_KeyMap[Action] = SKey(key);
}

input::EKeyCode CameraControl::GetKeyCode(EAction Action) const
{
	return m_KeyMap[Action].keyCode;
}

bool CameraControl::OnEvent(const input::Event& event)
{
	if(event.type == input::EEventType::Button) {
		for(int i = 0; i < EA_COUNT; ++i) {
			if(m_KeyMap[i].keyCode == event.button.code) {
				m_KeyMap[i].state = event.button.pressedDown;
				break;
			}
		}
	} else if(event.type == input::EEventType::Area &&
		event.source == input::EEventSource::Mouse &&
		event.area.code == input::AREA_MOUSE) {
		m_MouseMove.Set((float)event.area.relX, (float)event.area.relY);
	}

	return false;
}

bool CameraControl::IsKeyDown(EAction Action) const
{
	return m_KeyMap[Action].state;
}

float CameraControl::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

void CameraControl::SetMoveSpeed(float fSpeed)
{
	m_MoveSpeed = fSpeed;
}

math::anglef CameraControl::GetRotationSpeed() const
{
	return m_RotSpeed;
}

void CameraControl::SetRotationSpeed(math::anglef fSpeed)
{
	m_RotSpeed = fSpeed;
}

bool CameraControl::VerticalMovementAllowed() const
{
	return !m_NoVerticalMovement;
}

void CameraControl::AllowVerticalMovement(bool Allow)
{
	m_NoVerticalMovement = !Allow;
}

core::Name CameraControl::GetReferableSubType() const
{
	return SceneComponentType::CameraControl;
}

void CameraControl::OnAttach(Node* node)
{
	m_MouseMove.Set(0.0f, 0.0f);
	Animator::OnAttach(node);
}

}
}
