#ifndef INCLUDED_LUX_INPUT_SYSTEM_H
#define INCLUDED_LUX_INPUT_SYSTEM_H
#include "core/ReferenceCounted.h"
#include "core/lxSignal.h"
#include "input/InputEvent.h"
#include "input/InputDevice.h"

#include "core/lxHashMap.h"
#include "core/lxString.h"

namespace lux
{
namespace input
{

//! The input system of the lux engine.
class InputSystem : public ReferenceCounted
{
public:
	//! Initialize the global referable factory
	LUX_API static void Initialize();

	//! Access the global referable factory
	LUX_API static InputSystem* Instance();

	//! Destroys the global referable factory
	LUX_API static void Destroy();

	LUX_API InputSystem();
	LUX_API ~InputSystem();

	//! Get the event signal
	/**
	All input events created by the system are broadcast to this event
	*/
	LUX_API core::Signal<const Event&>& GetEventSignal();

	//! Set the current foreground state of the active window
	/**
	This method is called automaically by the engine.
	*/
	LUX_API void SetForegroundState(bool isForeground);

	//! Get the current foreground state of the input system.
	LUX_API bool IsForeground() const;

	//! Set the foreground handling for devices.
	/**
	\param isForeground The new foreground handling state.
	*/
	LUX_API void SetForegroundHandling(bool isForeground);

	//! Get the current default foreground handling state.
	LUX_API bool GetForegroundHandling() const;

	//! Sent a input event.
	/**
	This method will update a devices associated with the given event.
	This method can be used to emulate inputs.
	\param [in] [out] The event to send. If there is data missing in the event it will be filled if possible.
	*/
	LUX_API void Update(Event& event);

	//! Find the device belonging to a device description.
	/**
	\param desc The description of the input device
	\return The new input device.
	*/
	LUX_API StrongRef<InputDevice> FindDevice(InputDeviceDesc* desc);

	//! Get the primary keyboard
	LUX_API StrongRef<InputDevice> GetKeyboard();

	//! Get the primary mouse
	LUX_API StrongRef<InputDevice> GetMouse();

private:
	core::HashMap<core::String, StrongRef<InputDevice>> m_GUIDMap;
	WeakRef<InputDevice> m_KeyboardDevice;
	WeakRef<InputDevice> m_MouseDevice;

	core::Signal<const input::Event&> m_EventSignal;

	bool m_IsForeground;
	bool m_ForegroundHandling;
};

} // namespace input
} // namespace lux

#endif // #ifndef INCLUDED_LUX_INPUT_SYSTE_H