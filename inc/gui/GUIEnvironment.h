#ifndef INCLUDED_GUIENVIRONMENT_H
#define INCLUDED_GUIENVIRONMENT_H
#include "core/ReferenceCounted.h"
#include "core/lxName.h"
#include "math/Vector2.h"
#include "events/lxEvent.h"
#include "events/lxSignal.h"
#include "input/Keycodes.h"

#include "gui/GUIScalarDistance.h"

namespace lux
{
namespace input
{
class Event;
}
namespace gui
{
class Font;
class FontCreator;
class Element;
class Skin;
class Renderer;
class Cursor;
class Window;

class StaticText;
class Button;

class Event : public event::Event
{
public:
	Element* elem;
};

class MouseEvent : public Event
{
public:
	// Move, Click, Wheel, double click, etc.
	enum EType
	{
		Move,

		LDown,
		LUp,

		RDown,
		RUp,
	};

	EType type;
	math::Vector2F pos;
	bool leftState:1;
	bool rightState:1;
	bool ctrl:1;
	bool shift:1;
};

class KeyboardEvent : public Event
{
public:
	u32 character[4];
	input::EKeyCode code;
	bool down:1;
	bool autoRepeat:1;
	bool ctrl:1;
	bool shift:1;
};

class ElementEvent : public Event
{
public:
	enum EType
	{
		FocusGained,
		FocusLost,

		MouseEnter,
		MouseLeave
	};

	EType type;
};

class ControlEvent
{
	// ChangeFocus, etc.
};

//! Represent all parts of the GUI
class GUIEnvironment : public ReferenceCounted
{
public:
	LUX_API GUIEnvironment(Window* osWindow, Cursor* osCursor);
	LUX_API ~GUIEnvironment();

	LUX_API StrongRef<Window> GetRootElement();
	LUX_API StrongRef<Cursor> GetCursor();

	LUX_API void Update(float secsPassed);
	LUX_API void Render();

	LUX_API StrongRef<Skin> GetSkin() const;
	LUX_API void SetSkin(Skin* skin);

	LUX_API StrongRef<Renderer> GetRenderer() const;
	LUX_API void SetRenderer(Renderer* r);

	LUX_API StrongRef<Element> GetElementByPos(const math::Vector2F& pos);

	//! Retrieve the font creator.
	LUX_API StrongRef<FontCreator> GetFontCreator();

	/**
	Funktions to perform gui actions.
	Like:
	Change focus
	Click focused element
	Rightclick focused element
	Send custom events into the environment
	**/
	LUX_API void SendEvent(Element* elem, const Event& event);

	LUX_API Element* GetHovered();
	LUX_API Element* GetFocused();

	LUX_API void OnElementRemoved(Element* elem);

	///////////////////////////////////////////////////////////////////////////

	LUX_API StrongRef<Element> AddElement(core::Name name, Element* parent);
	LUX_API StrongRef<StaticText> AddStaticText(const ScalarVectorF& position, const String& text, Element* parent=nullptr);
	LUX_API StrongRef<Button> AddButton(const ScalarVectorF& pos, const ScalarDimensionF& size, const String& text, Element* parent=nullptr);

	///////////////////////////////////////////////////////////////////////////
private:
	void OnCursorMove(const math::Vector2F& newPos);
	void OnEvent(const input::Event& event);

	void SendMouseEvent(MouseEvent& e);

private:
	StrongRef<FontCreator> m_FontCreator;

	StrongRef<Window> m_Root;
	StrongRef<Renderer> m_Renderer;
	StrongRef<Skin> m_Skin;

	StrongRef<Cursor> m_OSCursor;
	StrongRef<Window> m_OSWindow;

	StrongRef<Cursor> m_Cursor;

	WeakRef<Element> m_Hovered;
	WeakRef<Element> m_Focused;

	math::Vector2F m_CursorPos;
	bool m_LeftState;
	bool m_RightState;
	bool m_ControlState;
	bool m_ShiftState;
};

} // namespace gui
} // namespace lux

#endif // INCLUDED_GUIENVIRONMENT_H
