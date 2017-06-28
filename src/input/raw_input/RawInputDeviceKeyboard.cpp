#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceKeyboard.h"

namespace lux
{
namespace input
{

RawKeyboardDevice::RawKeyboardDevice(InputSystem* system, HANDLE rawHandle) :
	RawInputDevice(system),
	m_DeadKey(0)
{
	memset(m_Win32KeyStates, 0, sizeof(m_Win32KeyStates));

	m_Name = "GenericKeyboard";
	m_GUID = GetDeviceGUID(rawHandle);

	RID_DEVICE_INFO info = GetDeviceInfo(rawHandle);
	if(info.dwType != RIM_TYPEKEYBOARD)
		throw core::InvalidArgumentException("rawHandle", "Is not a keyboard");
}

void RawKeyboardDevice::HandleInput(RAWINPUT* input)
{
	Event event;
	event.type = EEventType::Button;
	event.source = EEventSource::Keyboard;
	event.button.code = (EKeyCode)VKeyCodeToKeyCode(input->data.keyboard.VKey);
	event.button.state = ((input->data.keyboard.Flags & 1) == 0);
	event.button.pressedDown = event.button.state;
	event.internal_abs_only = false;
	event.internal_rel_only = false;

	if(event.button.code == -1)
		throw core::RuntimeException("Unknown key code");

	bool isControl = false;
	if(event.button.code == 143) {
		if(input->data.keyboard.MakeCode == 0x2a)
			event.button.code = EKeyCode::KEY_LSHIFT;
		else
			event.button.code = EKeyCode::KEY_RSHIFT;
		isControl = true;
	}

	if(event.button.code == 143 || event.button.code == 144 || event.button.code == 145) {
		bool right = ((input->data.keyboard.Flags & 2) != 0);
		if(event.button.code == 143)
			event.button.code = right ? EKeyCode::KEY_RSHIFT : EKeyCode::KEY_LSHIFT;
		if(event.button.code == 144)
			event.button.code = right ? EKeyCode::KEY_RCONTROL : EKeyCode::KEY_LCONTROL;
		if(event.button.code == 145)
			event.button.code = right ? EKeyCode::KEY_RMENU : EKeyCode::KEY_LMENU;
		isControl = true;
	}

	if(!isControl) {
		wchar_t BUFFER[10];
		GetKeyCharacter(input->data.keyboard, BUFFER, 10);
		if(BUFFER[0] != 0) {
			const char* cur = (const char*)BUFFER;
			event.keyInput.character[0] = core::AdvanceCursorUTF16(cur);
			event.keyInput.character[1] = core::AdvanceCursorUTF16(cur);
			event.keyInput.character[2] = 0;
		} else {
			event.keyInput.character[0] = 0;
		}
	} else {
		event.keyInput.character[0] = 0;
	}

	SendInputEvent(event);
}

EEventSource RawKeyboardDevice::GetType() const
{
	return EEventSource::Keyboard;
}

size_t RawKeyboardDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return 256;
	else
		return 0;
}

RawInputDevice::ElemDesc RawKeyboardDevice::GetElementDesc(EEventType type, u32 code) const
{
	LUX_UNUSED(code);
	LUX_UNUSED(type);
	static const string name = "(unknown)";

	return ElemDesc(name, 0, 0, EElementType::Other);
}

u32 RawKeyboardDevice::VKeyCodeToKeyCode(u16 code)
{
	switch(code) {
	case 0x07: return (u32)EKeyCode::KEY_CANCEL;
	case 0x08: return (u32)EKeyCode::KEY_BACK;
	case 0x09: return (u32)EKeyCode::KEY_TAB;
	case 0x0C: return (u32)EKeyCode::KEY_CLEAR;
	case 0x0D: return (u32)EKeyCode::KEY_RETURN;
	case 0x10: return 143;
	case 0x11: return 144;
	case 0x12: return 145;
	case 0x13: return (u32)EKeyCode::KEY_PAUSE;
	case 0x14: return (u32)EKeyCode::KEY_CAPITAL;
	case 0x15: return (u32)EKeyCode::KEY_HANGUL;
	case 0x17: return (u32)EKeyCode::KEY_JUNJA;
	case 0x18: return (u32)EKeyCode::KEY_FINAL;
	case 0x19: return (u32)EKeyCode::KEY_HANJA;
	case 0x1B: return (u32)EKeyCode::KEY_ESCAPE;
	case 0x1C: return (u32)EKeyCode::KEY_CONVERT;
	case 0x1D: return (u32)EKeyCode::KEY_NONCONVERT;
	case 0x1E: return (u32)EKeyCode::KEY_ACCEPT;
	case 0x1F: return (u32)EKeyCode::KEY_MODECHANGE;
	case 0x20: return (u32)EKeyCode::KEY_SPACE;
	case 0x21: return (u32)EKeyCode::KEY_PRIOR;
	case 0x22: return (u32)EKeyCode::KEY_NEXT;
	case 0x23: return (u32)EKeyCode::KEY_END;
	case 0x24: return (u32)EKeyCode::KEY_HOME;
	case 0x25: return (u32)EKeyCode::KEY_LEFT;
	case 0x26: return (u32)EKeyCode::KEY_UP;
	case 0x27: return (u32)EKeyCode::KEY_RIGHT;
	case 0x28: return (u32)EKeyCode::KEY_DOWN;
	case 0x29: return (u32)EKeyCode::KEY_SELECT;
	case 0x2A: return (u32)EKeyCode::KEY_PRINT;
	case 0x2B: return (u32)EKeyCode::KEY_EXECUT;
	case 0x2C: return (u32)EKeyCode::KEY_SNAPSHOT;
	case 0x2D: return (u32)EKeyCode::KEY_INSERT;
	case 0x2E: return (u32)EKeyCode::KEY_DELETE;
	case 0x2F: return (u32)EKeyCode::KEY_HELP;
	case 0x30: return (u32)EKeyCode::KEY_KEY_0;
	case 0x31: return (u32)EKeyCode::KEY_KEY_1;
	case 0x32: return (u32)EKeyCode::KEY_KEY_2;
	case 0x33: return (u32)EKeyCode::KEY_KEY_3;
	case 0x34: return (u32)EKeyCode::KEY_KEY_4;
	case 0x35: return (u32)EKeyCode::KEY_KEY_5;
	case 0x36: return (u32)EKeyCode::KEY_KEY_6;
	case 0x37: return (u32)EKeyCode::KEY_KEY_7;
	case 0x38: return (u32)EKeyCode::KEY_KEY_8;
	case 0x39: return (u32)EKeyCode::KEY_KEY_9;
	case 0x41: return (u32)EKeyCode::KEY_KEY_A;
	case 0x42: return (u32)EKeyCode::KEY_KEY_B;
	case 0x43: return (u32)EKeyCode::KEY_KEY_C;
	case 0x44: return (u32)EKeyCode::KEY_KEY_D;
	case 0x45: return (u32)EKeyCode::KEY_KEY_E;
	case 0x46: return (u32)EKeyCode::KEY_KEY_F;
	case 0x47: return (u32)EKeyCode::KEY_KEY_G;
	case 0x48: return (u32)EKeyCode::KEY_KEY_H;
	case 0x49: return (u32)EKeyCode::KEY_KEY_I;
	case 0x4A: return (u32)EKeyCode::KEY_KEY_J;
	case 0x4B: return (u32)EKeyCode::KEY_KEY_K;
	case 0x4C: return (u32)EKeyCode::KEY_KEY_L;
	case 0x4D: return (u32)EKeyCode::KEY_KEY_M;
	case 0x4E: return (u32)EKeyCode::KEY_KEY_N;
	case 0x4F: return (u32)EKeyCode::KEY_KEY_O;
	case 0x50: return (u32)EKeyCode::KEY_KEY_P;
	case 0x51: return (u32)EKeyCode::KEY_KEY_Q;
	case 0x52: return (u32)EKeyCode::KEY_KEY_R;
	case 0x53: return (u32)EKeyCode::KEY_KEY_S;
	case 0x54: return (u32)EKeyCode::KEY_KEY_T;
	case 0x55: return (u32)EKeyCode::KEY_KEY_U;
	case 0x56: return (u32)EKeyCode::KEY_KEY_V;
	case 0x57: return (u32)EKeyCode::KEY_KEY_W;
	case 0x58: return (u32)EKeyCode::KEY_KEY_X;
	case 0x59: return (u32)EKeyCode::KEY_KEY_Y;
	case 0x5A: return (u32)EKeyCode::KEY_KEY_Z;
	case 0x5B: return (u32)EKeyCode::KEY_LWIN;
	case 0x5C: return (u32)EKeyCode::KEY_RWIN;
	case 0x5D: return (u32)EKeyCode::KEY_APPS;
	case 0x5F: return (u32)EKeyCode::KEY_SLEEP;
	case 0x60: return (u32)EKeyCode::KEY_NUMPAD0;
	case 0x61: return (u32)EKeyCode::KEY_NUMPAD1;
	case 0x62: return (u32)EKeyCode::KEY_NUMPAD2;
	case 0x63: return (u32)EKeyCode::KEY_NUMPAD3;
	case 0x64: return (u32)EKeyCode::KEY_NUMPAD4;
	case 0x65: return (u32)EKeyCode::KEY_NUMPAD5;
	case 0x66: return (u32)EKeyCode::KEY_NUMPAD6;
	case 0x67: return (u32)EKeyCode::KEY_NUMPAD7;
	case 0x68: return (u32)EKeyCode::KEY_NUMPAD8;
	case 0x69: return (u32)EKeyCode::KEY_NUMPAD9;
	case 0x6A: return (u32)EKeyCode::KEY_MULTIPLY;
	case 0x6B: return (u32)EKeyCode::KEY_ADD;
	case 0x6C: return (u32)EKeyCode::KEY_SEPARATOR;
	case 0x6D: return (u32)EKeyCode::KEY_SUBTRACT;
	case 0x6E: return (u32)EKeyCode::KEY_DECIMAL;
	case 0x6F: return (u32)EKeyCode::KEY_DIVIDE;
	case 0x70: return (u32)EKeyCode::KEY_F1;
	case 0x71: return (u32)EKeyCode::KEY_F2;
	case 0x72: return (u32)EKeyCode::KEY_F3;
	case 0x73: return (u32)EKeyCode::KEY_F4;
	case 0x74: return (u32)EKeyCode::KEY_F5;
	case 0x75: return (u32)EKeyCode::KEY_F6;
	case 0x76: return (u32)EKeyCode::KEY_F7;
	case 0x77: return (u32)EKeyCode::KEY_F8;
	case 0x78: return (u32)EKeyCode::KEY_F9;
	case 0x79: return (u32)EKeyCode::KEY_F10;
	case 0x7A: return (u32)EKeyCode::KEY_F11;
	case 0x7B: return (u32)EKeyCode::KEY_F12;
	case 0x7C: return (u32)EKeyCode::KEY_F13;
	case 0x7D: return (u32)EKeyCode::KEY_F14;
	case 0x7E: return (u32)EKeyCode::KEY_F15;
	case 0x7F: return (u32)EKeyCode::KEY_F16;
	case 0x80: return (u32)EKeyCode::KEY_F17;
	case 0x81: return (u32)EKeyCode::KEY_F18;
	case 0x82: return (u32)EKeyCode::KEY_F19;
	case 0x83: return (u32)EKeyCode::KEY_F20;
	case 0x84: return (u32)EKeyCode::KEY_F21;
	case 0x85: return (u32)EKeyCode::KEY_F22;
	case 0x86: return (u32)EKeyCode::KEY_F23;
	case 0x87: return (u32)EKeyCode::KEY_F24;
	case 0x90: return (u32)EKeyCode::KEY_NUMLOCK;
	case 0x91: return (u32)EKeyCode::KEY_SCROLL;
	case 0xA0: return (u32)EKeyCode::KEY_LSHIFT;
	case 0xA1: return (u32)EKeyCode::KEY_RSHIFT;
	case 0xA2: return (u32)EKeyCode::KEY_LCONTROL;
	case 0xA3: return (u32)EKeyCode::KEY_RCONTROL;
	case 0xA4: return (u32)EKeyCode::KEY_LMENU;
	case 0xA5: return (u32)EKeyCode::KEY_RMENU;
	case 0xBA: return (u32)EKeyCode::KEY_OEM_1;
	case 0xBB: return (u32)EKeyCode::KEY_PLUS;
	case 0xBC: return (u32)EKeyCode::KEY_COMMA;
	case 0xBD: return (u32)EKeyCode::KEY_MINUS;
	case 0xBE: return (u32)EKeyCode::KEY_PERIOD;
	case 0xBF: return (u32)EKeyCode::KEY_OEM_2;
	case 0xC0: return (u32)EKeyCode::KEY_OEM_3;
	case 0xDB: return (u32)EKeyCode::KEY_OEM_4;
	case 0xDC: return (u32)EKeyCode::KEY_OEM_5;
	case 0xDD: return (u32)EKeyCode::KEY_OEM_6;
	case 0xDE: return (u32)EKeyCode::KEY_OEM_7;
	case 0xDF: return (u32)EKeyCode::KEY_OEM_8;
	case 0xE1: return (u32)EKeyCode::KEY_OEM_AX;
	case 0xE2: return (u32)EKeyCode::KEY_OEM_102;
	case 0xF6: return (u32)EKeyCode::KEY_ATTN;
	case 0xF7: return (u32)EKeyCode::KEY_CRSEL;
	case 0xF8: return (u32)EKeyCode::KEY_EXSEL;
	case 0xF9: return (u32)EKeyCode::KEY_EREOF;
	case 0xFA: return (u32)EKeyCode::KEY_PLAY;
	case 0xFB: return (u32)EKeyCode::KEY_ZOOM;
	case 0xFD: return (u32)EKeyCode::KEY_PA1;
	case 0xFE: return (u32)EKeyCode::KEY_OEM_CLEAR;
	}

	return (u32)EKeyCode::KEY_NONE;
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

void RawKeyboardDevice::GetKeyCharacter(RAWKEYBOARD& input,
	wchar_t* character, u32 maxSize)
{
	const HKL keyboardLayout = GetKeyboardLayout(0);

	int conversionResult;
	const UINT scanCode = input.MakeCode;
	const UINT vKeyCode = input.VKey;
	m_Win32KeyStates[VK_CAPITAL] = (((GetKeyState(VK_CAPITAL) & 0x0001) != 0) ? 0x81 : 0x00);
	m_Win32KeyStates[VK_RSHIFT] = (BYTE)GetKeyState(VK_RSHIFT); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_LSHIFT] = (BYTE)GetKeyState(VK_LSHIFT);// Upper byte contains only push button information.
	m_Win32KeyStates[VK_SHIFT] = m_Win32KeyStates[VK_LSHIFT] | m_Win32KeyStates[VK_RSHIFT];

	m_Win32KeyStates[VK_RMENU] = (BYTE)GetKeyState(VK_RMENU); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_LMENU] = (BYTE)GetKeyState(VK_LMENU); // Upper byte contains only push button information.
	m_Win32KeyStates[VK_MENU] = m_Win32KeyStates[VK_LMENU] | m_Win32KeyStates[VK_RMENU];

	UINT flags = (m_Win32KeyStates[VK_MENU] & 0x80) != 0 ? 1 : 0;
	wchar_t translatedKey[10];
	conversionResult = ToUnicodeEx(
		vKeyCode,
		scanCode,
		m_Win32KeyStates,
		translatedKey,
		sizeof(translatedKey) / sizeof(*translatedKey),
		flags,
		keyboardLayout);

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
	// TODO: Convert spacing dead-keys to non-spacing dead-keys.
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
