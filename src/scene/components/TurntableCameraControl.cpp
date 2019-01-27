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
{
}

void TurntableCameraControl::Animate(float time)
{
	auto node = GetNode();
	if(!node)
		return;

	float minDistance = 0.1f;
	auto pos = node->GetPosition();
	auto orbitVector = pos - m_OrbitCenter;
	if(math::IsZero(orbitVector.GetLengthSq()))
		orbitVector = -minDistance * math::Vector3F::UNIT_Z;
	auto orbitDistance = orbitVector.GetLength();
	auto flank = node->FromRelativeDir(math::Vector3F::UNIT_X, node->IsInheritingRotation() ? node->GetParent() : nullptr);

	math::QuaternionF rotY(m_TableNormal, m_RotSpeed * math::AngleF(m_RotDelta.y + time * m_RotRate.y));
	math::QuaternionF rotX(flank, m_RotSpeed * math::AngleF(m_RotDelta.x + time * m_RotRate.x));
	auto rot = rotX * rotY;

	auto newOrbit = rot.Transform(orbitVector);
	auto newOrientation = node->GetOrientation() * rot;
	float newDistance = orbitDistance - m_ZoomSpeed * (m_ZoomDelta + time * m_ZoomRate);
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
	if(event.device->GetType() == input::EDeviceType::Mouse) {
		if(!m_IsControlActive)
			return;

		if(auto area = event.TryAs<input::AreaEvent>()) {
			if(area->code == input::MOUSE_AREA) {
				RotXDelta(area->rel.y);
				RotYDelta(area->rel.x);
			}
		} else if(auto axis = event.TryAs<input::AxisEvent>()) {
			if(axis->code == input::MOUSE_AXIS_WHEEL) {
				ZoomDelta(axis->rel);
			}
		}
	} else if(event.device->GetType() == input::EDeviceType::Keyboard) {
		if(auto button = event.TryAs<input::ButtonEvent>()) {
			float value = button->pressedDown ? 1.0f : -1.0f;
			switch(button->code) {
			case input::KEY_LEFT:
				m_InputVector.y += value;
				break;
			case input::KEY_RIGHT:
				m_InputVector.y -= value;
				break;
			case input::KEY_DOWN:
				m_InputVector.x -= value;
				break;
			case input::KEY_UP:
				m_InputVector.x += value;
				break;
			case input::KEY_ADD:
			case input::KEY_PLUS:
				m_InputVector.z += value;
				break;
			case input::KEY_SUBTRACT:
			case input::KEY_MINUS:
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