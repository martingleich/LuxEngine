#include "scene/components/CameraControl.h"
#include "scene/Node.h"
#include "core/ReferableRegister.h"
#include "core/Logger.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::SceneComponentType::CameraControl, lux::scene::CameraControl)

namespace lux
{
namespace scene
{

CameraControl::CameraControl() :
	CameraControl(4.0f, math::anglef::Degree(30.0f), math::anglef::Degree(89.0f), false)
{
}

CameraControl::CameraControl(float moveSpeed, math::anglef rotSpeed, math::anglef maxAngle, bool noVertical) :
	m_MoveSpeed(moveSpeed),
	m_RotSpeed(rotSpeed),
	m_MaxVerticalAngle(maxAngle),
	m_NoVerticalMovement(noVertical)
{
}

CameraControl::~CameraControl()
{
}

void CameraControl::Animate(Node* node, float time)
{
	if(!m_Forward) {
		m_Forward = events::ActionList::Instance()->GetAxisAction("cam_forward");
		m_Flank = events::ActionList::Instance()->GetAxisAction("cam_stride");
		m_Up = events::ActionList::Instance()->GetAxisAction("cam_up");

		auto lookX = events::ActionList::Instance()->GetAxisAction("cam_look_x");
		if(lookX)
			lookX->signal.Connect(this, &CameraControl::MouseMoveX);
		auto lookY = events::ActionList::Instance()->GetAxisAction("cam_look_y");
		if(lookY)
			lookY->signal.Connect(this, &CameraControl::MouseMoveY);
	}

	math::vector3f pos = node->GetAbsolutePosition();
	math::vector3f flank = node->FromRelativeDir(math::vector3f::UNIT_X);
	math::vector3f up = node->FromRelativeDir(math::vector3f::UNIT_Y);
	math::vector3f look = node->FromRelativeDir(math::vector3f::UNIT_Z);

	math::vector3f move = look;
	if(m_NoVerticalMovement) {
		move.y = 0.0f;
		move.Normalize();
	}

	auto deltaRotY = m_MouseMove.x * m_RotSpeed;
	auto deltaRotX = -m_MouseMove.y * m_RotSpeed;

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

	if(m_Forward)
		pos += move * m_Forward->GetState() * m_MoveSpeed * time;

	if(m_Flank)
		pos += flank * m_Flank->GetState() * m_MoveSpeed * time;

	if(!m_NoVerticalMovement && m_Up)
		pos.y += m_Up->GetState() * m_MoveSpeed * time;

	node->SetPosition(pos);

	m_MouseMove.Set(0, 0);
}

float CameraControl::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

void CameraControl::SetMoveSpeed(float fSpeed)
{
	m_MoveSpeed = fSpeed;
}

void CameraControl::SetMaxVerticalAngle(math::anglef a)
{
	m_MaxVerticalAngle = a;
}
math::anglef CameraControl::GetMaxVerticalAngle() const
{
	return m_MaxVerticalAngle;
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

core::Name CameraControl::GetReferableType() const
{
	return SceneComponentType::CameraControl;
}

StrongRef<Referable> CameraControl::Clone() const
{
	return LUX_NEW(CameraControl)(*this);
}

void CameraControl::DefaultEventToCameraAction(const input::Event& event)
{
	static WeakRef<events::AxisAction> lookX = events::ActionList::Instance()->GetAxisAction("cam_look_x");
	static WeakRef<events::AxisAction> lookY = events::ActionList::Instance()->GetAxisAction("cam_look_y");

	static struct
	{
		input::EKeyCode a, b;
		WeakRef<events::AxisAction> action;
	} ENTRIES[] = {
		{input::KEY_KEY_W, input::KEY_KEY_S, events::ActionList::Instance()->GetAxisAction("cam_forward")},
		{input::KEY_KEY_D, input::KEY_KEY_A, events::ActionList::Instance()->GetAxisAction("cam_stride")},
		{input::KEY_KEY_Q, input::KEY_KEY_E, events::ActionList::Instance()->GetAxisAction("cam_up")},
	};

	if(event.type == input::EEventType::Button && event.source == input::EEventSource::Keyboard) {
		float value = event.button.state ? 1.0f : -1.0f;
		for(auto it = ENTRIES; it != ENTRIES + 3; ++it) {
			float v = it->action->GetState();
			if(event.button.code == it->a)
				v += value;
			if(event.button.code == it->b)
				v -= value;
			if(v != it->action->GetState())
				it->action->FireAxis(v);
		}
	} else if(event.type == input::EEventType::Area && event.source == input::EEventSource::Mouse) {
		if(lookX)
			lookX->FireAxis(event.area.relX);
		if(lookY)
			lookY->FireAxis(event.area.relY);
	}
}
void CameraControl::MouseMoveX(float v)
{
	m_MouseMove.x += v;
}

void CameraControl::MouseMoveY(float v)
{
	m_MouseMove.y += v;
}

}
}
