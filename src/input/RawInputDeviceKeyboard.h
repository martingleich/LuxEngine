#ifndef INCLUDED_RAW_KEYBOARD_DEVICE_H
#define INCLUDED_RAW_KEYBOARD_DEVICE_H
#include "RawInputDevice.h"

namespace lux
{
namespace input
{

class RawKeyboardDevice : public RawInputDevice
{
public:
	RawKeyboardDevice(InputSystem* system);
	EResult Init(HANDLE rawHandle);
	EResult HandleInput(RAWINPUT* input);
	EEventSource GetType() const;
	size_t GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, u32 code) const;

private:
	u32 VKeyCodeToKeyCode(u16 code);
	void GetKeyCharacter(RAWKEYBOARD& input, wchar_t* character, u32 maxSize);
	void TranslateCharacter(wchar_t c1, wchar_t c2, wchar_t* out, u32 maxSize);

private:
	BYTE m_Win32KeyStates[256];
	wchar_t m_DeadKey;
};

}
}

#endif // !INCLUDED_RAW_KEYBOARD_DEVICE_H