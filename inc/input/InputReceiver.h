#ifndef INCLUDED_EVENTRECEIVER_H
#define INCLUDED_EVENTRECEIVER_H
#include "input/InputEvent.h"

namespace lux
{
namespace input
{

//! An event receiver
/**
Every time the engine generates a event OnEvent is called
You can register your eventreceiver with LuxDevice::SetUserEventReceiver
or GUIEnvironment::SetUserEventReceiver
or use one of the many interfaces which implements this interface
See the respective documentation to learn more
*/
class InputReceiver
{
public:
	virtual ~InputReceiver()
	{
	}

	//! Split the path of a event
	/**
	Depending on the value of the event, one of the other receiver methods is called,
	i.e. OnKeyDown, OnKeyUp, OnMouseMove etc.
	*/
	virtual bool SplitEvent(const Event& event)
	{
		if(event.source == EEventSource::Keyboard) {
			if(event.type == EEventType::Button) {
				if(event.button.pressedDown)
					return OnKeyDown(event.button.code, event);
				else
					return OnKeyUp(event.button.code, event);
			}
		}

		if(event.source == EEventSource::Mouse) {
			if(event.type == EEventType::Button) {
				if(event.button.pressedDown)
					return OnMouseKeyDown(event.button.code, event);
				else
					return OnMouseKeyUp(event.button.code, event);
			}
			if(event.type == EEventType::Area) {
				return OnMouseMove(event.area.relX, event.area.relY, event);
			}

			if(event.type == EEventType::Axis) {
				if(event.axis.code == AXIS_MOUSE_WHEEL)
					return OnMouseWheel(event.axis.rel, event);
				if(event.axis.code == AXIS_MOUSE_HWHEEL)
					return OnMouseHWheel(event.axis.rel, event);
			}
		}

		if(event.source == EEventSource::Joystick)
			return OnJoystick(event);

		return OnUnknownEvent(event);
	}

	//! This is called for each event
	/**
	\param event The newly generated event
	\return Return true to signal that the event is fully handled
	it then is _not_ handed to the next receiver
	*/
	virtual bool OnEvent(const Event& event)
	{
		return SplitEvent(event);
	}

	virtual bool OnKeyDown(EKeyCode keyCode, const Event& event) { LUX_UNUSED(keyCode); LUX_UNUSED(event); return false; }
	virtual bool OnKeyUp(EKeyCode keyCode, const Event& event) {  LUX_UNUSED(keyCode); LUX_UNUSED(event); return false; }
	virtual bool OnMouseKeyDown(EKeyCode keyCode, const Event& event) { LUX_UNUSED(keyCode); LUX_UNUSED(event); return false; }
	virtual bool OnMouseKeyUp(EKeyCode keyCode, const Event& event) { LUX_UNUSED(keyCode); LUX_UNUSED(event); return false; }
	virtual bool OnMouseMove(float x, float y, const Event& event) { LUX_UNUSED(x); LUX_UNUSED(y); LUX_UNUSED(event); return false; }
	virtual bool OnMouseWheel(float move, const Event& event) { LUX_UNUSED(move); LUX_UNUSED(event); return false; }
	virtual bool OnMouseHWheel(float move, const Event& event) { LUX_UNUSED(move); LUX_UNUSED(event); return false; }
	virtual bool OnJoystick(const Event& event) { LUX_UNUSED(event); return false; }
	virtual bool OnUnknownEvent(const Event& event) { LUX_UNUSED(event); return false; }
};

}
}

#endif