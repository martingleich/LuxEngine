#include "InputDeviceJoystick.h"
#include "input/InputSystem.h"
#include "core/StringConverter.h"

namespace lux
{
namespace input
{

JoystickDevice::JoystickDevice(const DeviceCreationDesc* desc, InputSystem* system) :
	InputDeviceNull(desc->GetName(), system)
{
	m_Buttons.Resize(desc->GetElementCount(EEventType::Button));
	m_Axes.Resize(desc->GetElementCount(EEventType::Axis));

	for(size_t i = 0; i < m_Buttons.Size(); ++i) {
		Button& button = m_Buttons[i];
		auto elemDesc = desc->GetElementDesc(EEventType::Button, (u32)i);
		button.name = elemDesc.name;
		button.type = elemDesc.type;

		if(button.name.IsEmpty())
			button.name = "Button-" + core::StringConverter::ToString(i);
	}

	for(size_t i = 0; i < m_Axes.Size(); ++i) {
		Axis& axis = m_Axes[i];
		auto elemDesc = desc->GetElementDesc(EEventType::Axis, (u32)i);
		axis.name = elemDesc.name;
		axis.type = elemDesc.type;

		if(axis.name.IsEmpty())
			axis.name = "Axis-" + core::StringConverter::ToString(i);
	}

	Reset();
}

void JoystickDevice::Reset()
{
	for(auto it = m_Buttons.First(); it != m_Buttons.End(); ++it)
		it->state = false;

	for(auto it = m_Axes.First(); it != m_Axes.End(); ++it)
		it->state = 0;
}

void JoystickDevice::DisconnectReporting(InputSystem* system)
{
	Event event;
	event.device = this;
	event.source = EEventSource::Joystick;
	event.internal_abs_only = false;
	event.internal_rel_only = false;

	event.type = EEventType::Button;
	event.button.pressedDown = false;
	event.button.state = false;
	for(size_t i = 0; i < m_Buttons.Size(); ++i) {
		if(m_Buttons[i].state && !TestFlag(m_Buttons[i].type, EElementType::Rel)) {
			event.button.code = (EKeyCode)(u32)i;
			system->SendUserEvent(event);
		}
	}

	event.type = EEventType::Axis;
	event.axis.abs = 0;
	for(size_t i = 0; i < m_Axes.Size(); ++i) {
		if(m_Axes[i].state && !TestFlag(m_Axes[i].type, EElementType::Rel)) {
			event.axis.code = (EAxisCode)(u32)i;
			event.axis.rel = -m_Axes[i].state;
			system->SendUserEvent(event);
		}
	}
}

EEventSource JoystickDevice::GetType() const
{
	return EEventSource::Joystick;
}

bool JoystickDevice::GetButtonState(u32 buttonCode) const
{
	if(buttonCode >= m_Buttons.Size())
		return false;

	return m_Buttons[buttonCode].state;
}

int JoystickDevice::GetAxisState(u32 axisCode) const
{
	if(axisCode >= m_Axes.Size())
		return false;

	return m_Axes[axisCode].state;
}

math::vector2i JoystickDevice::GetAreaState(u32 areaCode) const
{
	LUX_UNUSED(areaCode);
	return math::vector2i::ZERO;
}

bool JoystickDevice::Update(Event& event)
{
	if(event.type == EEventType::Button) {
		if((size_t)event.button.code >= m_Buttons.Size())
			return false;

		if(m_Buttons[event.button.code].state != event.button.state) {
			m_Buttons[event.button.code].state = event.button.state;
			return true;
		}
		return false;
	}

	if(event.type == EEventType::Axis) {
		if((size_t)event.axis.code >= m_Axes.Size())
			return false;

		if(event.internal_abs_only)
			event.axis.rel = m_Axes[event.axis.code].state - event.axis.abs;
		else if(event.internal_rel_only)
			event.axis.abs = m_Axes[event.axis.code].state + event.axis.rel;

		if(m_Axes[event.axis.code].state != event.axis.abs) {
			m_Axes[event.axis.code].state = event.axis.abs;
			return true;
		}

		return false;
	}

	return false;
}

const string& JoystickDevice::GetElementName(EEventType type, u32 code) const
{
	static string unknown = "(unknown)";
	if(type == EEventType::Button && code < m_Buttons.Size())
		return m_Buttons[code].name;

	if(type == EEventType::Axis && code < m_Axes.Size())
		return m_Axes[code].name;

	return unknown;
}

EElementType JoystickDevice::GetElementType(EEventType type, u32 id) const
{
	if(type == EEventType::Button) {
		if(id < m_Buttons.Size())
			return m_Buttons[id].type;
	} else if(type == EEventType::Axis) {
		if(id < m_Axes.Size())
			return m_Axes[id].type;
	}

	return EElementType::Other;
}

size_t JoystickDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return m_Buttons.Size();
	else if(type == EEventType::Axis)
		return m_Axes.Size();

	return 0;
}

}
}