#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceKeyboard.h"

namespace lux
{
namespace input
{

static int VKeyCodeToKeyCode(const RAWKEYBOARD& keyboard)
{
	bool right = ((keyboard.Flags & 2) != 0);
	switch(keyboard.VKey) {
	case 0x07: return KEY_CANCEL;
	case 0x08: return KEY_BACK;
	case 0x09: return KEY_TAB;
	case 0x0C: return KEY_CLEAR;
	case 0x0D: return KEY_RETURN;
	case 0x10: return keyboard.MakeCode == 0x2a ? KEY_LSHIFT : KEY_RSHIFT;
	case 0x11: return right ? KEY_RCONTROL : KEY_LCONTROL;
	case 0x12: return right ? KEY_RMENU : KEY_LMENU;
	case 0x13: return KEY_PAUSE;
	case 0x14: return KEY_CAPITAL;
	case 0x15: return KEY_HANGUL;
	case 0x17: return KEY_JUNJA;
	case 0x18: return KEY_FINAL;
	case 0x19: return KEY_HANJA;
	case 0x1B: return KEY_ESCAPE;
	case 0x1C: return KEY_CONVERT;
	case 0x1D: return KEY_NONCONVERT;
	case 0x1E: return KEY_ACCEPT;
	case 0x1F: return KEY_MODECHANGE;
	case 0x20: return KEY_SPACE;
	case 0x21: return KEY_PRIOR;
	case 0x22: return KEY_NEXT;
	case 0x23: return KEY_END;
	case 0x24: return KEY_HOME;
	case 0x25: return KEY_LEFT;
	case 0x26: return KEY_UP;
	case 0x27: return KEY_RIGHT;
	case 0x28: return KEY_DOWN;
	case 0x29: return KEY_SELECT;
	case 0x2A: return KEY_PRINT;
	case 0x2B: return KEY_EXECUT;
	case 0x2C: return KEY_SNAPSHOT;
	case 0x2D: return KEY_INSERT;
	case 0x2E: return KEY_DELETE;
	case 0x2F: return KEY_HELP;
	case 0x30: return KEY_KEY_0;
	case 0x31: return KEY_KEY_1;
	case 0x32: return KEY_KEY_2;
	case 0x33: return KEY_KEY_3;
	case 0x34: return KEY_KEY_4;
	case 0x35: return KEY_KEY_5;
	case 0x36: return KEY_KEY_6;
	case 0x37: return KEY_KEY_7;
	case 0x38: return KEY_KEY_8;
	case 0x39: return KEY_KEY_9;
	case 0x41: return KEY_KEY_A;
	case 0x42: return KEY_KEY_B;
	case 0x43: return KEY_KEY_C;
	case 0x44: return KEY_KEY_D;
	case 0x45: return KEY_KEY_E;
	case 0x46: return KEY_KEY_F;
	case 0x47: return KEY_KEY_G;
	case 0x48: return KEY_KEY_H;
	case 0x49: return KEY_KEY_I;
	case 0x4A: return KEY_KEY_J;
	case 0x4B: return KEY_KEY_K;
	case 0x4C: return KEY_KEY_L;
	case 0x4D: return KEY_KEY_M;
	case 0x4E: return KEY_KEY_N;
	case 0x4F: return KEY_KEY_O;
	case 0x50: return KEY_KEY_P;
	case 0x51: return KEY_KEY_Q;
	case 0x52: return KEY_KEY_R;
	case 0x53: return KEY_KEY_S;
	case 0x54: return KEY_KEY_T;
	case 0x55: return KEY_KEY_U;
	case 0x56: return KEY_KEY_V;
	case 0x57: return KEY_KEY_W;
	case 0x58: return KEY_KEY_X;
	case 0x59: return KEY_KEY_Y;
	case 0x5A: return KEY_KEY_Z;
	case 0x5B: return KEY_LWIN;
	case 0x5C: return KEY_RWIN;
	case 0x5D: return KEY_APPS;
	case 0x5F: return KEY_SLEEP;
	case 0x60: return KEY_NUMPAD0;
	case 0x61: return KEY_NUMPAD1;
	case 0x62: return KEY_NUMPAD2;
	case 0x63: return KEY_NUMPAD3;
	case 0x64: return KEY_NUMPAD4;
	case 0x65: return KEY_NUMPAD5;
	case 0x66: return KEY_NUMPAD6;
	case 0x67: return KEY_NUMPAD7;
	case 0x68: return KEY_NUMPAD8;
	case 0x69: return KEY_NUMPAD9;
	case 0x6A: return KEY_MULTIPLY;
	case 0x6B: return KEY_ADD;
	case 0x6C: return KEY_SEPARATOR;
	case 0x6D: return KEY_SUBTRACT;
	case 0x6E: return KEY_DECIMAL;
	case 0x6F: return KEY_DIVIDE;
	case 0x70: return KEY_F1;
	case 0x71: return KEY_F2;
	case 0x72: return KEY_F3;
	case 0x73: return KEY_F4;
	case 0x74: return KEY_F5;
	case 0x75: return KEY_F6;
	case 0x76: return KEY_F7;
	case 0x77: return KEY_F8;
	case 0x78: return KEY_F9;
	case 0x79: return KEY_F10;
	case 0x7A: return KEY_F11;
	case 0x7B: return KEY_F12;
	case 0x7C: return KEY_F13;
	case 0x7D: return KEY_F14;
	case 0x7E: return KEY_F15;
	case 0x7F: return KEY_F16;
	case 0x80: return KEY_F17;
	case 0x81: return KEY_F18;
	case 0x82: return KEY_F19;
	case 0x83: return KEY_F20;
	case 0x84: return KEY_F21;
	case 0x85: return KEY_F22;
	case 0x86: return KEY_F23;
	case 0x87: return KEY_F24;
	case 0x90: return KEY_NUMLOCK;
	case 0x91: return KEY_SCROLL;
	case 0xA0: return KEY_LSHIFT;
	case 0xA1: return KEY_RSHIFT;
	case 0xA2: return KEY_LCONTROL;
	case 0xA3: return KEY_RCONTROL;
	case 0xA4: return KEY_LMENU;
	case 0xA5: return KEY_RMENU;
	case 0xBA: return KEY_OEM_1;
	case 0xBB: return KEY_PLUS;
	case 0xBC: return KEY_COMMA;
	case 0xBD: return KEY_MINUS;
	case 0xBE: return KEY_PERIOD;
	case 0xBF: return KEY_OEM_2;
	case 0xC0: return KEY_OEM_3;
	case 0xDB: return KEY_OEM_4;
	case 0xDC: return KEY_OEM_5;
	case 0xDD: return KEY_OEM_6;
	case 0xDE: return KEY_OEM_7;
	case 0xDF: return KEY_OEM_8;
	case 0xE1: return KEY_OEM_AX;
	case 0xE2: return KEY_OEM_102;
	case 0xF6: return KEY_ATTN;
	case 0xF7: return KEY_CRSEL;
	case 0xF8: return KEY_EXSEL;
	case 0xF9: return KEY_EREOF;
	case 0xFA: return KEY_PLAY;
	case 0xFB: return KEY_ZOOM;
	case 0xFD: return KEY_PA1;
	case 0xFE: return KEY_OEM_CLEAR;
	}

	return -1;
}

static u32 KeyToComb(u32 c)
{
	if(c == '^')
		return 0x302;
	else if(c == 180) // Accent ´
		return 0x301;
	else if(c == '`')
		return 0x300;
	return c;
}

static u32 CombToKey(u32 c)
{
	if(c == 0x302)
		return '^';
	else if(c == 0x301)
		return 180;
	else if(c == 0x300)
		return '`';
	return c;
}

RawKeyboardDevice::RawKeyboardDevice(InputSystem* system, HANDLE rawHandle, HKL keyboardLayout) :
	RawInputDevice(system),
	m_DeadKey(0),
	m_KeyboardLayout(keyboardLayout)
{
	std::memset(m_Win32KeyStates, 0, sizeof(m_Win32KeyStates));

	m_Desc = LUX_NEW(RawInputDeviceDescription);
	m_Desc->type = EDeviceType::Keyboard;
	m_Desc->name = "GenericKeyboard";
	m_Desc->guid = GetDeviceGUID(rawHandle);

	m_Desc->buttonCount = MAX_KEY_COUNT;
	for(int i = 0; i < m_Desc->buttonCount; ++i) {
		m_Desc->desc.EmplaceBack(
			"", CombineFlags(EDeviceElementType::Input, EDeviceElementType::Button));
	}

	m_Desc->axesCount = 0;
	m_Desc->areasCount = 0;
}

void RawKeyboardDevice::HandleInput(RAWINPUT* input)
{
	KeyboardButtonEvent event;
	event.code = VKeyCodeToKeyCode(input->data.keyboard);
	event.pressedDown = ((input->data.keyboard.Flags & 1) == 0);
	if(event.code < 0)
		return;

	bool isControl = 
		event.code == KEY_LSHIFT || event.code == KEY_RSHIFT ||
		event.code == KEY_LCONTROL || event.code == KEY_RCONTROL ||
		event.code == KEY_LMENU || event.code == KEY_RMENU;

	if(!isControl) {
		wchar_t BUFFER[10];
		GetKeyCharacter(input->data.keyboard, BUFFER, 10);
		if(BUFFER[0] != 0) {
			const char* cur = (const char*)BUFFER;
			event.character[0] = core::AdvanceCursorUTF16(cur);
			event.character[1] = core::AdvanceCursorUTF16(cur);
			event.character[2] = 0;
		} else {
			event.character[0] = 0;
		}
	} else {
		event.character[0] = 0;
	}

	SendInputEvent(event);
}

void RawKeyboardDevice::GetKeyCharacter(RAWKEYBOARD& input,
	wchar_t* character, u32 maxSize)
{
	int conversionResult;
	const UINT scanCode = input.MakeCode;
	const UINT vKeyCode = input.VKey;
	m_Win32KeyStates[VK_CAPITAL] = (((GetKeyState(VK_CAPITAL) & 0x0001) != 0) ? 0x81 : 0x00);
	m_Win32KeyStates[VK_RSHIFT] = (BYTE)GetKeyState(VK_RSHIFT); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_LSHIFT] = (BYTE)GetKeyState(VK_LSHIFT);// Upper byte contains only push button information.
	m_Win32KeyStates[VK_RCONTROL] = (BYTE)GetKeyState(VK_RCONTROL); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_LCONTROL] = (BYTE)GetKeyState(VK_LCONTROL);// Upper byte contains only push button information.
	m_Win32KeyStates[VK_SHIFT] = m_Win32KeyStates[VK_LSHIFT] | m_Win32KeyStates[VK_RSHIFT];

	m_Win32KeyStates[VK_RMENU] = (BYTE)GetKeyState(VK_RMENU); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_LMENU] = (BYTE)GetKeyState(VK_LMENU); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_MENU] = m_Win32KeyStates[VK_LMENU] | m_Win32KeyStates[VK_RMENU];
	m_Win32KeyStates[VK_CONTROL] = m_Win32KeyStates[VK_LCONTROL] | m_Win32KeyStates[VK_RCONTROL];

	UINT flags = (m_Win32KeyStates[VK_MENU] & 0x80) != 0 ? 1 : 0;
	wchar_t translatedKey[10];
	conversionResult = ToUnicodeEx(
		vKeyCode,
		scanCode,
		m_Win32KeyStates,
		translatedKey,
		sizeof(translatedKey) / sizeof(*translatedKey),
		flags,
		m_KeyboardLayout);

	if(conversionResult == 0) {
		character[0] = 0;
	} else if(conversionResult == -1) {
		// Dead-Key character
		if(!m_DeadKey) {
			m_DeadKey = (wchar_t)KeyToComb(translatedKey[0]);
			character[0] = 0;
		} else {
			character[0] = (wchar_t)CombToKey(m_DeadKey);
			character[1] = (wchar_t)CombToKey(translatedKey[0]);
			character[2] = 0;
			m_DeadKey = 0;
		}
	} else if(conversionResult == 1) {
		if(!m_DeadKey) {
			character[0] = translatedKey[0];
			if(maxSize > 1)
				character[1] = 0;
		} else {
			TranslateCharacter(m_DeadKey, translatedKey[0], character, maxSize);
			m_DeadKey = 0;
		}
	} else {
		if(maxSize - 1 >= (u32)conversionResult) {
			for(int i = 0; i < conversionResult; ++i)
				character[i] = translatedKey[i];
			character[maxSize - 1] = 0;
		}
	}

	character[maxSize - 1] = 0;
}

void RawKeyboardDevice::TranslateCharacter(wchar_t c1, wchar_t c2, wchar_t* out, u32 maxSize)
{
	if(c2 == 0 || c1 == 0) {
		out[0] = 0;
		return;
	}

	if(c2 == ' ') {
		out[0] = (wchar_t)CombToKey(c1);
		out[1] = 0;
		return;
	}

	wchar_t srcStr[2] = {c2, c1};
	int result = NormalizeString(NormalizationKC, srcStr, 2, out, maxSize - 1);
	if(result <= 0)
		out[0] = 0;
	else
		out[result] = 0;

}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
