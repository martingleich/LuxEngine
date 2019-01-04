#include "input/InputDeviceJoystick.h"
#include "input/InputSystem.h"
#include "core/StringConverter.h"

namespace lux
{
namespace input
{

JoystickDevice::JoystickDevice(const DeviceCreationDesc* desc, InputSystem* system) :
	m_Name(desc->GetName()),
	m_System(system)
{
	m_Buttons.Resize(desc->GetElementCount(EDeviceEventType::Button));
	m_Axes.Resize(desc->GetElementCount(EDeviceEventType::Axis));

	for(int i = 0; i < m_Buttons.Size(); ++i) {
		auto& button = m_Buttons[i];
		auto elemDesc = desc->GetElementDesc(EDeviceEventType::Button, i);
		button.name = elemDesc.name;
		button.type = elemDesc.type;

		if(button.name.IsEmpty())
			button.name = "Button-" + core::StringConverter::ToString(i);
	}

	for(int i = 0; i < m_Axes.Size(); ++i) {
		auto& axis = m_Axes[i];
		auto elemDesc = desc->GetElementDesc(EDeviceEventType::Axis, i);
		axis.name = elemDesc.name;
		axis.type = elemDesc.type;

		if(axis.name.IsEmpty())
			axis.name = "Axis-" + core::StringConverter::ToString(i);
	}

	for(auto it = m_Buttons.First(); it != m_Buttons.End(); ++it)
		it->state = false;

	for(auto it = m_Axes.First(); it != m_Axes.End(); ++it)
		it->state = 0;
}

void JoystickDevice::Reset()
{
	ButtonEvent buttonEvent;
	buttonEvent.device = this;
	for(int i = 0; i < m_Buttons.Size(); ++i) {
		if(m_Buttons[i].state && !TestFlag(m_Buttons[i].type, EDeviceElementType::Rel)) {
			buttonEvent.code = static_cast<EKeyCode>(i);
			m_System->SendUserEvent(buttonEvent);
			m_Buttons[i].state = false;
		}
	}

	AxisEvent axisEvent;
	axisEvent.device = this;
	axisEvent.abs = 0;
	for(int i = 0; i < m_Axes.Size(); ++i) {
		if(m_Axes[i].state && !TestFlag(m_Axes[i].type, EDeviceElementType::Rel)) {
			axisEvent.code = static_cast<EAxisCode>(i);
			axisEvent.rel = -m_Axes[i].state;
			m_System->SendUserEvent(buttonEvent);
			m_Buttons[i].state = false;
		}
	}
}

void JoystickDevice::Disconnect(bool reset)
{
	if(reset)
		Reset();
}

void JoystickDevice::Connect()
{
	m_IsConnected = true;
}

bool JoystickDevice::IsConnected() const 
{
	return m_IsConnected;
}

EDeviceType JoystickDevice::GetType() const
{
	return EDeviceType::Joystick;
}

bool JoystickDevice::GetButton(int buttonCode) const
{
	return m_Buttons.At(buttonCode).state;
}

float JoystickDevice::GetAxis(int axisCode) const
{
	return m_Axes.At(axisCode).state;
}

math::Vector2F JoystickDevice::GetArea(int areaCode) const
{
	throw core::ArgumentOutOfRangeException("areaCode", 0, 0, areaCode);
}

bool JoystickDevice::Update(Event& event)
{
	if(auto button = event.TryAs<ButtonEvent>()) {
		if(m_Buttons[button->code].state != button->pressedDown) {
			m_Buttons[button->code].state = button->pressedDown;
			return true;
		}
		return false;
	} 
	if(auto axis = event.TryAs<AxisEvent>()) {
		if(event.internal_abs_only)
			event.axis.rel = m_Axes[event.axis.code].state - event.axis.abs;
		else if(event.internal_rel_only)
			event.axis.abs = m_Axes[event.axis.code].state + event.axis.rel;

		if(m_Axes[axis->code].state != axis->abs) {
			m_Axes[axis->code].state = axis->abs;
			return true;
		}

		return false;
	}

	return false;
}

const core::String& JoystickDevice::GetElementName(EDeviceEventType type, int code) const
{
	static core::String unknown = "(unknown)";
	if(type == EDeviceEventType::Button && code < m_Buttons.Size())
		return m_Buttons[code].name;

	if(type == EDeviceEventType::Axis && code < m_Axes.Size())
		return m_Axes[code].name;

	return unknown;
}

EDeviceElementType JoystickDevice::GetElementType(EDeviceEventType type, int id) const
{
	if(type == EDeviceEventType::Button) {
		if(id < m_Buttons.Size())
			return m_Buttons[id].type;
	} else if(type == EDeviceEventType::Axis) {
		if(id < m_Axes.Size())
			return m_Axes[id].type;
	}

	return EDeviceElementType::Other;
}

int JoystickDevice::GetElementCount(EDeviceEventType type) const
{
	if(type == EDeviceEventType::Button)
		return m_Buttons.Size();
	else if(type == EDeviceEventType::Axis)
		return m_Axes.Size();

	return 0;
}

}
}