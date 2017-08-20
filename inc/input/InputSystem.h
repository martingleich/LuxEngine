#ifndef INCLUDED_INPUT_SYSTEM_H
#define INCLUDED_INPUT_SYSTEM_H
#include "core/ReferenceCounted.h"
#include "input/InputEvent.h"
#include "events/lxSignal.h"
#include "InputDevice.h"

namespace lux
{
namespace input
{
class DeviceCreationDesc;

//! The input system of the lux engine.
class InputSystem : public ReferenceCounted
{
public:
	//! Initialize the global referable factory
	LUX_API static void Initialize(InputSystem* inputSystem=nullptr);

	//! Access the global referable factory
	LUX_API static InputSystem* Instance();

	//! Destroys the global referable factory
	LUX_API static void Destroy();

	//! Get the event signal
	/**
	All input events created by the system are broadcast to this event
	*/
	virtual event::Signal<const Event&>& GetEventSignal() = 0;

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

	//! Send a event directly to the user, without updating the devices.
	virtual void SendUserEvent(const Event& event) = 0;

	//! Create a new input device.
	/**
	\param desc The description of the input device
	\return The new input device.
	*/
	virtual StrongRef<InputDevice> CreateDevice(const DeviceCreationDesc* desc) = 0;

	//! Get the primary keyboard
	virtual StrongRef<InputDevice> GetKeyboard() = 0;

	//! Get the primary mouse
	virtual StrongRef<InputDevice> GetMouse() = 0;
};

} // namespace input
} // namespace lux

#endif // #ifndef INCLUDED_INPUT_SYSTE_H