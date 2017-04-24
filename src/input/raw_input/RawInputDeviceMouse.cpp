#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceMouse.h"

#ifndef RI_MOUSE_HWHEEL
#define RI_MOUSE_HWHEEL 0x800
#endif

namespace lux
{
namespace input
{

RawMouseDevice::RawMouseDevice(InputSystem* system) :
	RawInputDevice(system),
	m_ButtonCount(8),
	m_HasHWheel(false)
{
}

EResult RawMouseDevice::Init(HANDLE rawHandle)
{
	m_Name = "GenericMouse";
	GetDeviceGUID(rawHandle, m_GUID);

	RID_DEVICE_INFO info;
	if(Succeeded(GetDeviceInfo(rawHandle, info))) {
		if(info.dwType != RIM_TYPEMOUSE)
			return EResult::Failed;
		m_ButtonCount = info.mouse.dwNumberOfButtons;
		m_HasHWheel = (info.mouse.fHasHorizontalWheel == TRUE);
	}

	for(int i = 0; i < ARRAYSIZE(m_ButtonState); ++i)
		m_ButtonState[i] = false;

	return EResult::Succeeded;
}

EResult RawMouseDevice::HandleInput(RAWINPUT* input)
{
	RAWMOUSE& mouse = input->data.mouse;

	if(mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		SendPosEvent(false, mouse.lLastX, mouse.lLastY);
	else if(mouse.usFlags == 0)
		SendPosEvent(true, mouse.lLastX, mouse.lLastY);

	for(int i = 0; i < 5; ++i) {
		int down_flag = (1 << (2 * i));
		int up_flag = (1 << (2 * i + 1));

		if((mouse.usButtonFlags & down_flag) && !m_ButtonState[i])
			SendButtonEvent(i, true);

		if((mouse.usButtonFlags & up_flag) && m_ButtonState[i])
			SendButtonEvent(i, false);
	}

	if(mouse.usButtonFlags & RI_MOUSE_WHEEL)
		SendWheelEvent((short)mouse.usButtonData);

	if(mouse.usButtonFlags & RI_MOUSE_HWHEEL)
		SendHWheelEvent((short)mouse.usButtonData);

	return EResult::Succeeded;
}

EEventSource RawMouseDevice::GetType() const
{
	return EEventSource::Mouse;
}

size_t RawMouseDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return m_ButtonCount;
	else if(type == EEventType::Axis)
		return m_HasHWheel ? 2 : 1;
	else
		return 1;
}

RawInputDevice::ElemDesc RawMouseDevice::GetElementDesc(EEventType type, u32 code) const
{
	static const string button_names[] = {
		"Left Button",
		"Right Button",
		"Middle Button",
		"X1 Button",
		"X2 Button"};

	static const string axis_names[] = {
		"Wheel",
		"Horizontal Wheel"};

	static const string area_names[] = {
		"Position"};

	static const string unknown = "(unknown)";

	if(type == EEventType::Button) {
		if(code >= ARRAYSIZE(button_names))
			return ElemDesc(unknown, 0, 0, EElementType::Other);
		return ElemDesc(button_names[code], 0, 0, EElementType::Other);
	} else if(type == EEventType::Axis) {
		if(code >= ARRAYSIZE(axis_names))
			return ElemDesc(unknown, 0, 0, EElementType::Other);
		return ElemDesc(axis_names[code], 0, 0, EElementType::Other);
	} else if(type == EEventType::Area) {
		if(code >= ARRAYSIZE(area_names))
			return ElemDesc(unknown, 0, 0, EElementType::Other);
		return ElemDesc(area_names[code], 0, 0, EElementType::Other);
	} else {
		return ElemDesc(unknown, 0, 0, EElementType::Other);
	}
}

void RawMouseDevice::SendButtonEvent(u32 button, bool state)
{
	Event event;
	event.source = EEventSource::Mouse;
	event.type = EEventType::Button;
	event.internal_abs_only = false;
	event.internal_rel_only = false;

	event.button.code = (EKeyCode)GetKeyCodeFromVKey(button);
	event.button.pressedDown = state;
	event.button.state = event.button.pressedDown;

	m_ButtonState[button] = state;

	SendInputEvent(event);
}

void RawMouseDevice::SendPosEvent(bool relative, s32 x, s32 y)
{
	Event event;
	event.source = EEventSource::Mouse;
	event.type = EEventType::Area;
	event.internal_abs_only = !relative;
	event.internal_rel_only = relative;

	event.area.code = EAreaCode::AREA_MOUSE;
	if(relative) {
		event.area.relX = x;
		event.area.relY = y;
	} else {
		event.area.absX = x;
		event.area.absY = y;
	}

	SendInputEvent(event);
}

void RawMouseDevice::SendWheelEvent(s32 move)
{
	Event event;
	event.source = EEventSource::Mouse;
	event.type = EEventType::Axis;
	event.internal_abs_only = false;
	event.internal_rel_only = true;

	event.axis.code = EAxisCode::AXIS_MOUSE_WHEEL;
	event.axis.rel = move;

	SendInputEvent(event);
}

void RawMouseDevice::SendHWheelEvent(s32 move)
{
	if(!m_HasHWheel)
		m_HasHWheel = true;

	Event event;
	event.source = EEventSource::Mouse;
	event.type = EEventType::Axis;
	event.internal_abs_only = false;
	event.internal_rel_only = true;

	event.axis.code = EAxisCode::AXIS_MOUSE_HWHEEL;
	event.axis.rel = move;

	SendInputEvent(event);
}

u32 RawMouseDevice::GetKeyCodeFromVKey(u32 vkey)
{
	switch(vkey) {
	case 0: return (u32)EKeyCode::KEY_LBUTTON;
	case 1: return (u32)EKeyCode::KEY_RBUTTON;
	case 2: return (u32)EKeyCode::KEY_MBUTTON;
	case 3: return (u32)EKeyCode::KEY_X1BUTTON;
	case 4: return (u32)EKeyCode::KEY_X2BUTTON;
	default: return (u32)EKeyCode::KEY_NONE;
	}
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
