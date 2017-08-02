#ifndef INCLUDED_RAW_MOUSE_DEVICE_H
#define INCLUDED_RAW_MOUSE_DEVICE_H

#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDevice.h"

namespace lux
{
namespace input
{

class RawMouseDevice : public RawInputDevice
{
public:
	RawMouseDevice(InputSystem* system, HANDLE rawHandle);

	void HandleInput(RAWINPUT* input);
	EEventSource GetType() const;
	size_t GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, u32 code) const;

private:
	void SendButtonEvent(u32 button, bool state);
	void SendPosEvent(bool relative, s32 x, s32 y);
	void SendWheelEvent(s32 move);
	void SendHWheelEvent(s32 move);

	static u32 GetKeyCodeFromVKey(u32 vkey);

	// Raw input supports at max 5 mouse buttons.
	static const size_t MAX_MOUSE_BUTTONS = 5;
private:
	bool m_ButtonStates[MAX_MOUSE_BUTTONS];

	size_t m_ButtonCount;
	bool m_HasHWheel;
};

}
}


#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_MOUSE_DEVICE_H