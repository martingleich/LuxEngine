#ifndef INCLUDED_LUX_KEYCODES_H
#define INCLUDED_LUX_KEYCODES_H

namespace lux
{
namespace input
{

//! Predefinend key codes
enum EKeyCode : int
{
	KEY_CANCEL = 0,  //!< Cancel
	KEY_BACK,  //!< Back
	KEY_TAB,  //!< Tabulator
	KEY_CLEAR,  //!< CLEAR key
	KEY_RETURN,  //!< Enter

	KEY_SHIFT,  //!< shift
	KEY_CONTROL,  //!< control
	KEY_MENU,  //!< Alternativ(Alt)

	KEY_PAUSE,  //!< Pause
	KEY_CAPITAL,  //!< Caps lock
	KEY_KANA,  //!< IME Kana Modus
	KEY_HANGUEL,  //!< IME Hanguel Modus
	KEY_HANGUL,  //!< IME Hangul Modus
	KEY_JUNJA,  //!< IME Junja Modus
	KEY_FINAL,  //!< IME final Modus
	KEY_HANJA,  //!< IME Hanja Modus
	KEY_KANJI,  //!< IME Kanji Modus
	KEY_ESCAPE,  //!< Escape
	KEY_CONVERT,  //!< IME convert
	KEY_NONCONVERT,  //!< IME nonconvert
	KEY_ACCEPT,  //!< IME accept
	KEY_MODECHANGE,  //!< IME Modus änderung
	KEY_SPACE,  //!< Space
	KEY_PRIOR,  //!< Page up
	KEY_NEXT,  //!< Page down
	KEY_END,  //!< Ende
	KEY_HOME,  //!< Home
	KEY_LEFT,  //!< Left arrowkey
	KEY_UP,  //!< Up arrowkey
	KEY_RIGHT,  //!< Right arrowkey
	KEY_DOWN,  //!< Down arrowkey
	KEY_SELECT,  //!< Select
	KEY_PRINT,  //!< Print
	KEY_EXECUT,  //!< Execute
	KEY_SNAPSHOT,  //!< Screenshot
	KEY_INSERT,  //!< Insert
	KEY_DELETE,  //!< Delete
	KEY_HELP,  //!< Help
	KEY_KEY_0,  //!< 0
	KEY_KEY_1,  //!< 1
	KEY_KEY_2,  //!< 2
	KEY_KEY_3,  //!< 3
	KEY_KEY_4,  //!< 4
	KEY_KEY_5,  //!< 5
	KEY_KEY_6,  //!< 6
	KEY_KEY_7,  //!< 7
	KEY_KEY_8,  //!< 8
	KEY_KEY_9,  //!< 9
	KEY_KEY_A,  //!< A
	KEY_KEY_B,  //!< B
	KEY_KEY_C,  //!< C
	KEY_KEY_D,  //!< D
	KEY_KEY_E,  //!< E
	KEY_KEY_F,  //!< F
	KEY_KEY_G,  //!< G
	KEY_KEY_H,  //!< H
	KEY_KEY_I,  //!< I
	KEY_KEY_J,  //!< J
	KEY_KEY_K,  //!< K
	KEY_KEY_L,  //!< L
	KEY_KEY_M,  //!< M
	KEY_KEY_N,  //!< N
	KEY_KEY_O,  //!< O
	KEY_KEY_P,  //!< P
	KEY_KEY_Q,  //!< Q
	KEY_KEY_R,  //!< R
	KEY_KEY_S,  //!< S
	KEY_KEY_T,  //!< T
	KEY_KEY_U,  //!< U
	KEY_KEY_V,  //!< V
	KEY_KEY_W,  //!< W
	KEY_KEY_X,  //!< X
	KEY_KEY_Y,  //!< Y
	KEY_KEY_Z,  //!< Z
	KEY_LWIN,  //!< Left Window key
	KEY_RWIN,  //!< Right Window key
	KEY_APPS,  //!< Apps key
	KEY_SLEEP,  //!< Sleep
	KEY_NUMPAD0,  //!< Numpad 0
	KEY_NUMPAD1,  //!< Numpad 1
	KEY_NUMPAD2,  //!< Numpad 2
	KEY_NUMPAD3,  //!< Numpad 3
	KEY_NUMPAD4,  //!< Numpad 4
	KEY_NUMPAD5,  //!< Numpad 5
	KEY_NUMPAD6,  //!< Numpad 6
	KEY_NUMPAD7,  //!< Numpad 7
	KEY_NUMPAD8,  //!< Numpad 8
	KEY_NUMPAD9,  //!< Numpad 9
	KEY_MULTIPLY,  //!< Multiply
	KEY_ADD,  //!< Add
	KEY_SEPARATOR,  //!< Seperator
	KEY_SUBTRACT,  //!< Subtract
	KEY_DECIMAL,  //!< Decimal comma
	KEY_DIVIDE,  //!< Divide
	KEY_F1,  //!< F1
	KEY_F2,  //!< F2
	KEY_F3,  //!< F3
	KEY_F4,  //!< F4
	KEY_F5,  //!< F5
	KEY_F6,  //!< F6
	KEY_F7,  //!< F7
	KEY_F8,  //!< F8
	KEY_F9,  //!< F9
	KEY_F10,  //!< F10
	KEY_F11,  //!< F11
	KEY_F12,  //!< F12
	KEY_F13,  //!< F13
	KEY_F14,  //!< F14
	KEY_F15,  //!< F15
	KEY_F16,  //!< F16
	KEY_F17,  //!< F17
	KEY_F18,  //!< F18
	KEY_F19,  //!< F19
	KEY_F20,  //!< F20
	KEY_F21,  //!< F21
	KEY_F22,  //!< F22
	KEY_F23,  //!< F23
	KEY_F24,  //!< F24
	KEY_NUMLOCK,  //!< Numlock
	KEY_SCROLL,  //!< Scrollock
	KEY_LSHIFT,  //!< Left shift
	KEY_RSHIFT,  //!< Right shift
	KEY_LCONTROL,  //!< Left control
	KEY_RCONTROL,  //!< Right control
	KEY_LMENU,  //!< Left menü
	KEY_RMENU,  //!< Right menü
	KEY_OEM_1,  //!< USA ";:"
	KEY_PLUS,  //!< Plus "+"
	KEY_COMMA,  //!< Komma ","
	KEY_MINUS,  //!< Minus "-"
	KEY_PERIOD,  //!< Punkt "."
	KEY_OEM_2,  //!< USA "/?"
	KEY_OEM_3,  //!< USA "`~"
	KEY_OEM_4,  //!< USA "[{"
	KEY_OEM_5,  //!< USA "\|"
	KEY_OEM_6,  //!< USA "]}"
	KEY_OEM_7,  //!< USA "'""
	KEY_OEM_8,  //!< Not used
	KEY_OEM_AX,  //!< Japan "AX"
	KEY_OEM_102,  //!< "<>" or "\|"
	KEY_ATTN,  //!< Attn
	KEY_CRSEL,  //!< CrSel
	KEY_EXSEL,  //!< ExSel
	KEY_EREOF,  //!< Remove end of file
	KEY_PLAY,  //!< Play
	KEY_ZOOM,  //!< Zoom
	KEY_PA1,  //!< PA1
	KEY_OEM_CLEAR,  //!< OEM Clear
	KEY_KEYBOARD_COUNT,
};

enum EMouseButtonCode : int
{
	MOUSE_BUTTON_LEFT = 0,  //!< Left mousebutton
	MOUSE_BUTTON_RIGHT,  //!< Right mousebutton
	MOUSE_BUTTON_MIDDLE,  //!< Middle mousebutton
	MOUSE_BUTTON_X1,  //!< X1 Mausknopf
	MOUSE_BUTTON_X2,  //!< X2 Mausknopf
};

enum EMouseAreaCode
{
	MOUSE_AREA = 0,    //!< The mouse position
};

//! Possible axis to send events
enum EMouseAxisCode : int
{
	MOUSE_AXIS_WHEEL = 0,  //! The mouse wheel
	MOUSE_AXIS_HWHEEL, //! The horizontal mouse wheel
};

}
}    //namespace lux

#endif

