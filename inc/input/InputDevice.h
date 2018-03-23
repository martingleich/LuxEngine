#ifndef INCLUDED_INPUT_DEVICE_H
#define INCLUDED_INPUT_DEVICE_H
#include "core/ReferenceCounted.h"
#include "input/InputEvent.h"
#include "core/lxActions.h"

namespace lux
{
namespace input
{
class InputSystem;

//! Internal class used for creating new input devices.
class DeviceCreationDesc : public ReferenceCounted
{
public:
	struct ElemDesc
	{
		ElemDesc(const core::String& _name, u16 _usagePage, u16 _usage, EElementType _type) :
			name(_name), usagePage(_usagePage), usage(_usage), type(_type)
		{
		}

		const core::String& name;

		u16 usagePage;
		u16 usage;

		EElementType type;
		int instance;
	};

public:
	virtual ~DeviceCreationDesc() {}

	virtual EEventSource GetType() const = 0;
	virtual const core::String& GetName() const = 0;
	virtual const core::String& GetGUID() const = 0;
	virtual int GetElementCount(EEventType type) const = 0;
	virtual ElemDesc GetElementDesc(EEventType type, int id) const = 0;
};

//! Class for a single input device.
/**
A input device is a collection of multiple Elements.
Each element is identified by its event type(i.e. Button, Axis, etc.) and
it's id.
*/
class InputDevice : public ReferenceCounted
{
public:
	virtual ~InputDevice() {}

	//! The type of the input device
	virtual EEventSource GetType() const = 0;

	//! The number of elements on this device
	/**
	\param type The event type given by the queried elements.
	\return The number of elments on the device.
	*/
	virtual int GetElementCount(EEventType type) const = 0;

	//! The name of a device element.
	/**
	Intended for display.
	\param type The eventtype of the element.
	\param id The id of the element
	\return The name of the element.
	*/
	virtual const core::String& GetElementName(EEventType type, int id) const = 0;

	//! The exact type of the device element.
	/**
	\param type The eventtype of the element.
	\param id The id of the element
	\return A or'ed combination of element types.
	*/
	virtual EElementType GetElementType(EEventType type, int id) const = 0;

	//! Get the state of a button
	/**
	\param buttonCode The id of the button.
	*/
	virtual const core::Button* GetButton(int buttonCode) const = 0;

	//! Get the state of an axis.
	/**
	\param axisCode The id of the axis.
	\return The value of the axis in device units.
	*/
	virtual const core::Axis* GetAxis(int axisCode) const = 0;

	//! Get the state of an area.
	/**
	\param areaCode The id of the area.
	\return The value of the area in device units.
	*/
	virtual const core::Area* GetArea(int areaCode) const = 0;

	//! Update the device with this event.
	/**
	Shouldn't be used directly by the user.
	To emulate input events use InputSystem::Update.
	\param [in] [out] event The event to change the device with,
	the device will fill missing information in the event(like absolute or relative movements).
	\return Was the state of the device changed by the event.
	*/
	virtual bool Update(Event& event) = 0;

	//! Reset all elements of the device to their default value.
	/**
	The device will create matching events.
	*/
	virtual void Reset() = 0;

	//! Is called when the device is disconnected from the input system
	virtual void DisconnectReporting(InputSystem* system) = 0;

	//! Get the name of the device.
	/**
	For user display.
	\return The device name.
	*/
	virtual const core::String& GetName() const = 0;

	//! Connect the device.
	virtual bool Connect() = 0;
	//! Disconnect the device
	/**
	The device will reconnect if it generates a event.
	*/
	virtual void Disconnect() = 0;

	//! Is the device connected.
	/**
	A connected device has sent data at least one time.
	*/
	virtual bool IsConnected() const = 0;

	//! Enable event handling for the device
	/**
	Only aquired device can sent data.
	*/
	virtual bool Aquire() = 0;

	//! Disable event handling for the device.
	/**
	The device won't sent data until it's aquired again.
	*/
	virtual void UnAquire() = 0;

	//! Is the device aquired.
	/**
	Only aquired device can sent data.
	*/
	virtual bool IsAquired() const = 0;

	//! Is the device a foreground device.
	/**
	A foreground device will only sent data if the current window is in the foreground.
	*/
	virtual bool IsForeground() const = 0;

	//! Configure the device.
	/**
	\param isForeground Is the device a foreground device.
	*/
	virtual void Configure(bool isForeground) = 0;
};

} // namespace input
} // namespace lux

#endif // #ifndef INCLUDED_INPUT_DEVICE_H