#ifndef INCLUDED_GUI_EVENT_H
#define INCLUDED_GUI_EVENT_H
#include "core/lxEvent.h"
#include "input/Keycodes.h"

namespace lux
{
namespace gui
{

class Element;

class Event : public event::Event
{
public:
	Element* elem;

	template <typename T>
	bool Is() const
	{
		static_assert(std::is_base_of<Event, T>::value, "T must be derived of gui::Event");
		return dynamic_cast<const T*>(this) != nullptr;
	}
	template <typename T>
	T& As()
	{
		static_assert(std::is_base_of<Event, T>::value, "T must be derived of gui::Event");
		auto ptr = dynamic_cast<T*>(this);
		if(!ptr)
			throw core::InvalidCastException();
		return *ptr;
	}
	template <typename T>
	const T& As() const
	{
		static_assert(std::is_base_of<Event, T>::value, "T must be derived of gui::Event");
		auto ptr = dynamic_cast<const T*>(this);
		if(!ptr)
			throw core::InvalidCastException();
		return *ptr;
	}
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

	bool IsClick() const
	{
		return type == LDown || type == RDown;
	}

	EType type;
	math::Vector2F pos;
	bool leftState : 1;
	bool rightState : 1;
	bool ctrl : 1;
	bool shift : 1;
};

class KeyboardEvent : public Event
{
public:
	u32 character[4];
	input::EKeyCode key;
	bool down : 1;
	bool autoRepeat : 1;
	bool ctrl : 1;
	bool shift : 1;
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

} // namespace gui
} // namespace lux

#endif // #ifndef INCLUDED_GUI_EVENT_H