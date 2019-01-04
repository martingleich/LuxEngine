#include "InputDeviceKeyboard.h"
#include "input/InputSystem.h"

namespace lux
{
namespace input
{

KeyboardDevice::KeyboardDevice(const DeviceCreationDesc* desc, InputSystem* system) :
	InputDeviceNull(desc->GetName(), system)
{
	m_Buttons.Resize(143);

	for(auto it = m_Buttons.First(); it != m_Buttons.End(); ++it)
		it->type = CombineFlags(EDeviceElementType::Input, EDeviceElementType::Button, EDeviceElementType::PushButton);

	Reset();
}

void KeyboardDevice::Reset()
{
	for(int i = 0; i < m_Buttons.Size(); ++i)
		m_Buttons[i].state = false;
}

void KeyboardDevice::DisconnectReporting(InputSystem* system)
{
	Event event;
	event.device = this;
	event.source = EDeviceType::Keyboard;
	event.internal_abs_only = false;
	event.internal_rel_only = false;
	event.type = EDeviceEventType::Button;
	event.button.pressedDown = false;
	event.button.state = false;
	for(int i = 0; i < m_Buttons.Size(); ++i) {
		if(m_Buttons[i].state) {
			event.button.code = (EKeyCode)i;
			system->SendUserEvent(event);
		}
	}
}

EDeviceType KeyboardDevice::GetType() const
{
	return EDeviceType::Keyboard;
}

const core::Button* KeyboardDevice::GetButton(int buttonCode) const
{
	return &m_Buttons.At(buttonCode);
}

const core::Axis* KeyboardDevice::GetAxis(int axisCode) const
{
	throw core::ArgumentOutOfRangeException("axisCode", 0,0, axisCode);
}

const core::Area* KeyboardDevice::GetArea(int areaCode) const
{
	throw core::ArgumentOutOfRangeException("areaCode", 0,0, areaCode);
}

bool KeyboardDevice::Update(Event& event)
{
	if(event.type != EDeviceEventType::Button || (int)event.button.code >= m_Buttons.Size())
		throw core::ArgumentOutOfRangeException("event.button.code", 0,m_Buttons.Size(), (int)event.button.code);

	if(m_Buttons[event.button.code].state != event.button.state) {
		m_Buttons[event.button.code].state = event.button.state;
		return true;
	}

	return false;
}

const core::String& KeyboardDevice::GetElementName(EDeviceEventType type, int code) const
{
	LUX_UNUSED(type);
	LUX_UNUSED(code);

	static core::String unknown = "(unknown)";
	return unknown;
}

EDeviceElementType KeyboardDevice::GetElementType(EDeviceEventType type, int id) const
{
	if(type == EDeviceEventType::Button) {
		if(id < m_Buttons.Size())
			return m_Buttons[id].type;
	}

	return EDeviceElementType::Other;
}

int KeyboardDevice::GetElementCount(EDeviceEventType type) const
{
	if(type == EDeviceEventType::Button)
		return m_Buttons.Size();
	return 0;
}

}
}