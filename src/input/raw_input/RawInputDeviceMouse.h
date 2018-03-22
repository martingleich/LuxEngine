#ifndef INCLUDED_RAW_MOUSE_DEVICE_H
#define INCLUDED_RAW_MOUSE_DEVICE_H

#include "LuxConfig.h"
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
	int GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, int code) const;

private:
	void SendButtonEvent(int button, bool state);
	void SendPosEvent(bool relative, int x, int y);
	void SendWheelEvent(int move);
	void SendHWheelEvent(int move);

	static EKeyCode GetKeyCodeFromVKey(int vkey);

	// Raw input supports at max 5 mouse buttons.
	// But we don't trust raw input.
	static const int MAX_MOUSE_BUTTONS = 10;
private:
	bool m_ButtonStates[MAX_MOUSE_BUTTONS];

	int m_ButtonCount;
	bool m_HasHWheel;
};

}
}


#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_MOUSE_DEVICE_H