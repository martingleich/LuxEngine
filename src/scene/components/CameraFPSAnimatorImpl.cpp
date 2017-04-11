#include "CameraFPSAnimatorImpl.h"
#include "scene/nodes/CameraSceneNode.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::CameraFPSAnimatorImpl)

namespace lux
{
namespace scene
{

CameraFPSAnimatorImpl::CameraFPSAnimatorImpl() :
	CameraFPSAnimatorImpl(1.0f, math::anglef::Radian(1.0f), math::anglef::Degree(89.0f), false)
{}

CameraFPSAnimatorImpl::CameraFPSAnimatorImpl(float fMoveSpeed, math::anglef fRotSpeed,
	math::anglef fMaxVerticalAngle, bool bNoVerticalMovement)
	: m_MoveSpeed(fMoveSpeed), m_RotSpeed(fRotSpeed), m_MaxVerticalAngle(fMaxVerticalAngle),
	m_NoVerticalMovement(bNoVerticalMovement), m_FirstCall(true), m_MouseMove(0.0f, 0.0f)
{
	this->SetEventReceiver(true);
	this->SetAnimated(true);

	m_KeyMap[EA_FORWARD] = input::KEY_KEY_W;
	m_KeyMap[EA_BACKWARD] = input::KEY_KEY_S;
	m_KeyMap[EA_LEFT] = input::KEY_KEY_A;
	m_KeyMap[EA_RIGHT] = input::KEY_KEY_D;
	m_KeyMap[EA_FAST] = input::KEY_LSHIFT;
	m_KeyMap[EA_SLOW] = input::KEY_LCONTROL;
	m_KeyMap[EA_UP] = input::KEY_KEY_Q;
	m_KeyMap[EA_DOWN] = input::KEY_KEY_E;
}

CameraFPSAnimatorImpl::~CameraFPSAnimatorImpl()
{}

void CameraFPSAnimatorImpl::Animate(float time)
{
	SceneNode* node = GetParent();
	if(!node)
		return;

	node->UpdateAbsTransform();

	CameraSceneNode* camNode = dynamic_cast<CameraSceneNode*>(node);
	if(!camNode)
		return;

	// Werte zwischenspeichern
	math::vector3f pos = camNode->GetAbsoluteTransform().translation;
	math::vector3f lookDir = camNode->FromRelativeDir(math::vector3f::UNIT_Z);

	math::vector2f MouseMove = m_MouseMove;
	math::vector3f vRelRot = lookDir.GetRotAngles();
	vRelRot.y += MouseMove.x * m_RotSpeed.Radian() * 0.001f;
	vRelRot.x += MouseMove.y * m_RotSpeed.Radian() * 0.001f;

	// Drehung begrenzen
	if(vRelRot.x > m_MaxVerticalAngle.Radian() * 2 &&
		vRelRot.x < math::Constants<float>::two_pi() - m_MaxVerticalAngle.Radian()) {
		vRelRot.x = math::Constants<float>::two_pi() - m_MaxVerticalAngle.Radian();
	} else if(vRelRot.x > m_MaxVerticalAngle.Radian() &&
		vRelRot.x < math::Constants<float>::two_pi() - m_MaxVerticalAngle.Radian()) {
		vRelRot.x = m_MaxVerticalAngle.Radian();
	}

	lookDir = math::vector3f(vRelRot.x, vRelRot.y, 0.0f).RotToDir();
	math::vector3f moveDir = lookDir;
	if(m_NoVerticalMovement)
		moveDir.y = 0.0f;

	// Bewegung verarbeiten
	math::vector3f vFlank = moveDir.Cross(camNode->GetUpVector()).Normalize_s();
	moveDir.Normalize();

	float tempSpeed = m_MoveSpeed;
	if(IsKeyDown(EA_FAST)) {
		tempSpeed *= 2.0f;
	}

	if(IsKeyDown(EA_SLOW)) {
		tempSpeed *= 0.5f;
	}

	if(IsKeyDown(EA_FORWARD))
		pos += moveDir * tempSpeed * time;

	if(IsKeyDown(EA_BACKWARD))
		pos -= moveDir * tempSpeed * time;

	if(IsKeyDown(EA_LEFT))
		pos += vFlank * tempSpeed * time;

	if(IsKeyDown(EA_RIGHT))
		pos -= vFlank * tempSpeed * time;

	if(m_NoVerticalMovement == false) {
		if(IsKeyDown(EA_UP))
			pos.y += tempSpeed * time;

		if(IsKeyDown(EA_DOWN))
			pos.y -= tempSpeed * time;
	}

	// Werte aktualisieren
	camNode->SetDirection(lookDir);
	camNode->SetPosition(pos);

	m_MouseMove.Set(0.0f, 0.0f);
}

StrongRef<Referable> CameraFPSAnimatorImpl::Clone() const
{
	StrongRef<CameraFPSAnimatorImpl> out = LUX_NEW(CameraFPSAnimatorImpl)(m_MoveSpeed, m_RotSpeed,
		m_MaxVerticalAngle, m_NoVerticalMovement);

	for(int i = 0; i < EA_COUNT; ++i)
		out->SetKeyCode((EAction)i, this->GetKeyCode((EAction)i));

	return out;
}

void CameraFPSAnimatorImpl::SetKeyCode(EAction Action, input::EKeyCode key)
{
	m_KeyMap[Action] = SKey(key);
}

input::EKeyCode CameraFPSAnimatorImpl::GetKeyCode(EAction Action) const
{
	return m_KeyMap[Action].keyCode;
}

void CameraFPSAnimatorImpl::SetActive(bool Active)
{
	if(IsActive() == false && Active == true)
		m_MouseMove.Set(0.0f, 0.0f);

	SceneNodeComponent::SetActive(Active);
}

bool CameraFPSAnimatorImpl::OnEvent(const input::Event& event)
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

bool CameraFPSAnimatorImpl::IsKeyDown(EAction Action) const
{
	return m_KeyMap[Action].state;
}

float CameraFPSAnimatorImpl::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

void CameraFPSAnimatorImpl::SetMoveSpeed(float fSpeed)
{
	m_MoveSpeed = fSpeed;
}

math::anglef CameraFPSAnimatorImpl::GetRotationSpeed() const
{
	return m_RotSpeed;
}

void CameraFPSAnimatorImpl::SetRotationSpeed(math::anglef fSpeed)
{
	m_RotSpeed = fSpeed;
}

bool CameraFPSAnimatorImpl::VerticalMovementAllowed() const
{
	return !m_NoVerticalMovement;
}

void CameraFPSAnimatorImpl::AllowVerticalMovement(bool Allow)
{
	m_NoVerticalMovement = !Allow;
}

core::Name CameraFPSAnimatorImpl::GetReferableSubType() const
{
	return SceneNodeComponentType::CameraFPS;
}

}    

}    

