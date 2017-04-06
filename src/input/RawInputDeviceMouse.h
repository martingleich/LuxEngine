#ifndef INCLUDED_RAW_MOUSE_DEVICE_H
#define INCLUDED_RAW_MOUSE_DEVICE_H
#include "RawInputDevice.h"

namespace lux
{
namespace input
{

class RawMouseDevice : public RawInputDevice
{
public:
	RawMouseDevice(InputSystem* system);

	EResult Init(HANDLE rawHandle);
	EResult HandleInput(RAWINPUT* input);
	EEventSource GetType() const;
	size_t GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, u32 code) const;

private:
	void SendButtonEvent(u32 button, bool state);
	void SendPosEvent(bool relative, s32 x, s32 y);
	void SendWheelEvent(s32 move);
	void SendHWheelEvent(s32 move);

	static u32 GetKeyCodeFromVKey(u32 vkey);

private:
	bool m_ButtonState[8];

	size_t m_ButtonCount;
	bool m_HasHWheel;
};

}
}

#endif // !INCLUDED_RAW_MOUSE_DEVICE_H