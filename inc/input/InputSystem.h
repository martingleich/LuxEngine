#ifndef INCLUDED_INPUT_SYSTEM_H
#define INCLUDED_INPUT_SYSTEM_H
#include "core/ReferenceCounted.h"
#include "core/Result.h"

namespace lux
{
namespace input
{
class InputDevice;
class EventReceiver;
class Event;
class DeviceCreationDesc;

// TODO:
/*
Establish control from InputSystem to RawInputReceiver(just a little bit)
*/

//! The input system of the lux engine.
class InputSystem : public ReferenceCounted
{
public:
	//! Set the input receiver used by the input system.
	/**
	The default input receiver is the engine-core.
	It should never be necessary to change this value.
	If it's changed the rest of the engine won't receiver input.
	*/
	virtual void SetInputReceiver(EventReceiver* receiver) = 0;

	//! Get the current input receiver.
	virtual EventReceiver* GetInputReceiver() const = 0;

	//! Set the current foreground state of the active window
	/**
	This method is called automaically by the engine.
	*/
	virtual void SetForegroundState(bool isForeground) = 0;

	//! Get the current foreground state of the input system.
	virtual bool IsForeground() const = 0;

	//! Set the default foreground handling for devices.
	/**
	Newly created devices will have the default foreground handling.
	All currenltly created devices which are in default state are changed to the new handling.
	\param isForeground The new foreground handling state.
	*/
	virtual void SetDefaultForegroundHandling(bool isForeground) = 0;

	//! Get the current default foreground handling state.
	virtual bool GetDefaultForegroundHandling() const = 0;

	//! Sent a input event.
	/**
	This method will update a devices associated with the given event.
	This method can be used to emulate inputs.
	\param [in] [out] The event to send. If there is data missing in the event it will be filled if possible.
	*/
	virtual void Update(Event& event) = 0;

	//! Sent a user event.
	/**
	This method only send's the event to the event receiver.
	To emulate user input use Update().
	*/
	virtual void SendUserEvent(const Event& event) = 0;

	//! Create a new input device.
	/**
	\param desc The description of the input device
	\return The new input device.
	*/
	virtual StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc) = 0;

	//! Get the primary keyboard
	virtual StrongRef<InputDevice> GetKeyboard() = 0;
};

}
}

#endif // #ifndef INCLUDED_INPUT_SYSTE_H