#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceMouse.h"

#ifndef RI_MOUSE_HWHEEL
#define RI_MOUSE_HWHEEL 0x800
#endif

static const float mouseNormalization = 100.0f;

// TODO: Normalize the mouse event size, 1.0 should equal round about the same movement

namespace lux
{
namespace input
{

RawMouseDevice::RawMouseDevice(InputSystem* system, HANDLE rawHandle) :
	RawInputDevice(system)
{
	auto desc = LUX_NEW(RawInputDeviceDescription);
	m_Desc = desc;
	desc->name = "GenericMouse";
	desc->guid = GetDeviceGUID(rawHandle);
	desc->type = EDeviceType::Mouse;

	RID_DEVICE_INFO info = GetDeviceInfo(rawHandle);
	if(info.dwType != RIM_TYPEMOUSE)
		throw core::GenericInvalidArgumentException("rawHandle", "Is not a mouse");

	static const core::String button_names[] = {
		"Left Button",
		"Right Button",
		"Middle Button",
		"X1 Button",
		"X2 Button",
		"Button 6",
		"Button 7",
		"Button 8",
		"Button 9",
		"Button 10"};

	static const core::String axis_names[] = {
		"Wheel",
		"Horizontal Wheel"};

	static const core::String area_names[] = {
		"Position"};

	desc->buttonCount = info.mouse.dwNumberOfButtons;
	// Force at least 3 buttons and a hwheel since windows is really bad at
	// reporting correct data
	if(desc->buttonCount < 3)
		desc->buttonCount = 3;
	if(desc->buttonCount > MAX_MOUSE_BUTTONS)
		desc->buttonCount = MAX_MOUSE_BUTTONS;
	for(int i = 0; i < desc->buttonCount; ++i) {
		desc->desc.EmplaceBack(
			button_names[i],
			CombineFlags(EDeviceElementType::Input, EDeviceElementType::Button));
	}

	// Techniqcally there is a flag for this, but more often than not it is wrong.
	desc->axesCount = 2; 
	for(int i = 0; i < desc->axesCount; ++i) { 
		desc->desc.EmplaceBack(
			axis_names[i],
			CombineFlags(EDeviceElementType::Input, EDeviceElementType::Axis, EDeviceElementType::Rel));
	}

	desc->areasCount = 1;
	for(int i = 0; i < desc->areasCount; ++i) { 
		desc->desc.EmplaceBack(
			area_names[i],
			CombineFlags(EDeviceElementType::Input, EDeviceElementType::Area, EDeviceElementType::Rel));
	}

	for(int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
		m_ButtonStates[i] = false;
}

void RawMouseDevice::HandleInput(RAWINPUT* input)
{
	RAWMOUSE& mouse = input->data.mouse;

	// TODO: Handle move absolute correctly.
	/*
	if(mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		SendPosEvent(false, mouse.lLastX, mouse.lLastY);
	else if(mouse.usFlags == 0)
	*/
	SendPosEvent(true, mouse.lLastX, mouse.lLastY);

	for(int i = 0; i < m_Desc->buttonCount; ++i) {
		int down_flag = (1 << (2 * i));
		int up_flag = (1 << (2 * i + 1));

		if((mouse.usButtonFlags & down_flag) && !m_ButtonStates[i])
			SendButtonEvent(i, true);

		if((mouse.usButtonFlags & up_flag) && m_ButtonStates[i])
			SendButtonEvent(i, false);
	}

	if(mouse.usButtonFlags & RI_MOUSE_WHEEL)
		SendWheelEvent((short)mouse.usButtonData);

	if(mouse.usButtonFlags & RI_MOUSE_HWHEEL)
		SendHWheelEvent((short)mouse.usButtonData);
}

StrongRef<InputDeviceDesc> RawMouseDevice::GetDescription()
{
	return m_Desc;
}

void RawMouseDevice::SendButtonEvent(int button, bool state)
{
	lxAssert(button < m_Desc->buttonCount);

	ButtonEvent event;
	event.code = GetKeyCodeFromVKey(button);
	if(event.code < 0)
		return;
	event.pressedDown = state;

	m_ButtonStates[button] = state;

	SendInputEvent(event);
}

void RawMouseDevice::SendPosEvent(bool relative, int x, int y)
{
	AreaEvent event;
	event.code = MOUSE_AREA;
	if(relative)
		event.rel.Set(x / mouseNormalization, y / mouseNormalization);
	else
		event.abs.Set(x / mouseNormalization, y / mouseNormalization);

	SendInputEvent(event);
}

void RawMouseDevice::SendWheelEvent(int move)
{
	AxisEvent event;
	event.code = MOUSE_AXIS_WHEEL;
	event.rel = (float)move;
	SendInputEvent(event);
}

void RawMouseDevice::SendHWheelEvent(int move)
{
	AxisEvent event;
	event.code = MOUSE_AXIS_HWHEEL;
	event.rel = (float)move;
	SendInputEvent(event);
}

int RawMouseDevice::GetKeyCodeFromVKey(int vkey)
{
	switch(vkey) {
	case 0: return MOUSE_BUTTON_LEFT;
	case 1: return MOUSE_BUTTON_RIGHT;
	case 2: return MOUSE_BUTTON_MIDDLE;
	case 3: return MOUSE_BUTTON_X1;
	case 4: return MOUSE_BUTTON_X2;
	default: return -1;
	}
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
