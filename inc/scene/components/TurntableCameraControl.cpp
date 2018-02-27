#include "scene/components/TurntableCameraControl.h"
#include "scene/Node.h"
#include "input/InputSystem.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::TurntableCameraControl, "lux.comp.TurntableCameraControl");

namespace lux
{
namespace scene
{

TurntableCameraControl::TurntableCameraControl()
{
	m_IsControlActive = true;
	m_TableNormal = math::Vector3F(0, 1, 0);
	m_ZoomRate = 0;
	m_ZoomDelta = 0;
	input::InputSystem::Instance()->GetEventSignal().Connect(this, &TurntableCameraControl::HandleInput);

	m_ZoomKeySpeed = 500.0f;
	m_ZoomSpeed = 0.005f;

	m_RotKeySpeed = 1.0f;
	m_RotSpeed = 1.0f;
}

TurntableCameraControl::~TurntableCameraControl()
{}

void TurntableCameraControl::Animate(float time)
{
	auto node = GetParent();
	if(!node)
		return;

	float minDistance = 0.1f;
	auto pos = node->GetPosition();
	auto orbitVector = pos - m_OrbitCenter;
	if(math::IsZero(orbitVector.GetLengthSq()))
		orbitVector = -minDistance * math::Vector3F::UNIT_Z;
	auto orbitDistance = orbitVector.GetLength();
	auto flank = node->FromRelativeDir(math::Vector3F::UNIT_X, node->GetParent());

	math::QuaternionF rotY(m_TableNormal, m_RotSpeed * math::AngleF(m_RotDelta.y + time*m_RotRate.y));
	math::QuaternionF rotX(flank, m_RotSpeed * math::AngleF(m_RotDelta.x + time*m_RotRate.x));
	auto rot = rotX * rotY;

	auto newOrbit = rot.Transform(orbitVector);
	auto newOrientation = node->GetOrientation() * rot;
	float newDistance = orbitDistance - m_ZoomSpeed * (m_ZoomDelta + time*m_ZoomRate);
	if(newDistance < minDistance)
		newDistance = minDistance;
	newOrbit.SetLength(newDistance);

	node->SetOrientation(newOrientation);
	node->SetPosition(m_OrbitCenter + newOrbit);

	m_RotDelta.Set(0, 0);
	m_ZoomDelta = 0;
}

void TurntableCameraControl::HandleInput(const input::Event& event)
{
	if(event.source == input::EEventSource::Mouse) {
		if(event.type == input::EEventType::Area) {
			if(m_IsControlActive) {
				RotXDelta(event.area.relY);
				RotYDelta(event.area.relX);
			}
		}
		if(event.type == input::EEventType::Axis) {
			if(event.axis.code == input::EAxisCode::AXIS_MOUSE_WHEEL) {
				if(m_IsControlActive) {
					ZoomDelta(event.axis.rel);
				}
			}
		}
	}

	if(event.source == input::EEventSource::Keyboard) {
		float value = event.button.pressedDown ? 1.0f : -1.0f;
		switch(event.button.code) {
		case input::EKeyCode::KEY_LEFT:
			m_InputVector.y += value;
			break;
		case input::EKeyCode::KEY_RIGHT:
			m_InputVector.y -= value;
			break;
		case input::EKeyCode::KEY_DOWN:
			m_InputVector.x -= value;
			break;
		case input::EKeyCode::KEY_UP:
			m_InputVector.x += value;
			break;
		case input::EKeyCode::KEY_ADD:
		case input::EKeyCode::KEY_PLUS:
			m_InputVector.z += value;
			break;
		case input::EKeyCode::KEY_SUBTRACT:
		case input::EKeyCode::KEY_MINUS:
			m_InputVector.z -= value;
			break;
		default:
			break;
		}
		if(m_IsControlActive) {
			RotXRate(m_InputVector.x * m_RotKeySpeed);
			RotYRate(m_InputVector.y * m_RotKeySpeed);
			ZoomRate(m_InputVector.z * m_ZoomKeySpeed);
		}
	}
}

void TurntableCameraControl::EnableInput()
{
	m_IsControlActive = true;
	RotXRate(m_InputVector.x);
	RotYRate(m_InputVector.y);
	ZoomRate(m_InputVector.z);
}

void TurntableCameraControl::DisableInput()
{
	RotXRate(0);
	RotYRate(0);
	ZoomRate(0);
	m_IsControlActive = false;
}

} // namespace scene
} // namespace lux