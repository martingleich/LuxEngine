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

	for(auto it = m_Buttons.First(); it != m_Buttons.End(); ++it) {
		it->type = EElementType::Input | EElementType::Button | EElementType::PushButton;
	}

	Reset();
}

void KeyboardDevice::Reset()
{
	for(size_t i = 0; i < m_Buttons.Size(); ++i)
		m_Buttons[i].state = false;
}

void KeyboardDevice::DisconnectReporting(InputSystem* system)
{
	Event event;
	event.device = this;
	event.source = EEventSource::Keyboard;
	event.internal_abs_only = false;
	event.internal_rel_only = false;
	event.type = EEventType::Button;
	event.button.pressedDown = false;
	event.button.state = false;
	for(size_t i = 0; i < m_Buttons.Size(); ++i) {
		if(m_Buttons[i].state) {
			event.button.code = (EKeyCode)(u32)i;
			system->SendUserEvent(event);
		}
	}
}

EEventSource KeyboardDevice::GetType() const
{
	return EEventSource::Keyboard;
}

bool KeyboardDevice::GetButtonState(u32 buttonCode) const
{
	if(buttonCode >= m_Buttons.Size())
		return false;

	return m_Buttons[buttonCode].state;
}

int KeyboardDevice::GetAxisState(u32 axisCode) const
{
	LUX_UNUSED(axisCode);
	return 0;
}

math::vector2i KeyboardDevice::GetAreaState(u32 areaCode) const
{
	LUX_UNUSED(areaCode);
	return math::vector2i::ZERO;
}

bool KeyboardDevice::Update(Event& event)
{
	if(event.type != EEventType::Button || (size_t)event.button.code >= m_Buttons.Size())
		return false;

	if(m_Buttons[event.button.code].state != event.button.state) {
		m_Buttons[event.button.code].state = event.button.state;
		return true;
	}

	return false;
}

const string& KeyboardDevice::GetElementName(EEventType type, u32 code) const
{
	LUX_UNUSED(type);
	LUX_UNUSED(code);

	static string unknown = "(unknown)";
	return unknown;
}

EElementType KeyboardDevice::GetElementType(EEventType type, u32 id) const
{
	if(type == EEventType::Button) {
		if(id < m_Buttons.Size())
			return m_Buttons[id].type;
	}

	return EElementType::Other;
}

size_t KeyboardDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return m_Buttons.Size();
	return 0;
}

}
}