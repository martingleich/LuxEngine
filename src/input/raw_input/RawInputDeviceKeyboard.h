#ifndef INCLUDED_RAW_KEYBOARD_DEVICE_H
#define INCLUDED_RAW_KEYBOARD_DEVICE_H

#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDevice.h"

namespace lux
{
namespace input
{

class RawKeyboardDevice : public RawInputDevice
{
public:
	RawKeyboardDevice(InputSystem* system, HANDLE rawHandle, HKL keyboardLayout);
	void HandleInput(RAWINPUT* input);
	EEventSource GetType() const;
	size_t GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, u32 code) const;

	void SetKeyboardLayout(HKL hkl);

private:
	u32 VKeyCodeToKeyCode(u16 code);
	void GetKeyCharacter(RAWKEYBOARD& input, wchar_t* character, u32 maxSize);
	void TranslateCharacter(wchar_t c1, wchar_t c2, wchar_t* out, u32 maxSize);

	static const size_t MAX_KEY_COUNT = 256; //!< There are only 256 virtual keys.
private:
	BYTE m_Win32KeyStates[MAX_KEY_COUNT];
	wchar_t m_DeadKey;

	HKL m_KeyboardLayout;
};

}
}

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_KEYBOARD_DEVICE_H