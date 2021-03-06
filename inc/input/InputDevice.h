#ifndef INCLUDED_LUX_INPUT_DEVICE_H
#define INCLUDED_LUX_INPUT_DEVICE_H
#include "core/ReferenceCounted.h"
#include "input/InputEvent.h"
#include "core/lxArray.h"

namespace lux
{
namespace input
{
class InputSystem;

//! From where a event was sent
enum class EDeviceType
{
	Mouse, //!< The event was generated by the mouse 
	Keyboard, //!< The event was generated by the keyboard
	Joystick, //!< The event was generated by a joystick.
};

//! What action generated an event
enum class EDeviceEventType
{
	//! A event was generated by changing a axis
	/*
	A linear value was changed
	*/
	Axis,

	//! A event was generated by changing a area
	/**
	A two dimensional value was changed
	*/
	Area,

	//! A event was generated by changing the state of a button
	/**
	A boolean value was changed
	*/
	Button
};

//! Flag class
enum class EDeviceElementType
{
	Input = 0x1,
	Output = 0x2,

	Axis = 0x4,
	Button = 0x8,

	Abs = 0x10,
	Rel = 0x20,

	PushButton = 0x40,
	ToggleButton = 0x80,

	POV = 0x100,

	ForceFeedback = 0x200,

	Area = 0x400,

	Other = 0x800
};

//! Internal class used for creating new input devices.
class InputDeviceDesc : public ReferenceCounted
{
public:
	virtual EDeviceType GetType() const = 0;
	virtual core::StringView GetName() const = 0;
	virtual core::StringView GetGUID() const = 0;

	virtual int GetElementCount(EDeviceEventType type) const = 0;
	virtual core::StringView GetElementName(EDeviceEventType type, int id) const = 0;	
	virtual EDeviceElementType GetElementType(EDeviceEventType type, int id) const = 0;	
};

//! Class for a single input device.
/**
A input device is a collection of multiple Elements.
Each element is identified by its event type(i.e. Button, Axis, etc.) and it's id.
*/
class InputDevice : public ReferenceCounted
{
public:
	LUX_API InputDevice(InputDeviceDesc* desc, InputSystem* system);
	LUX_API ~InputDevice();

	//! The type of the input device
	EDeviceType GetType() const { return m_Type; }

	//! Get the name of the device.
	/**
	For user display, i.e. Not meaningful or unique.
	\return The device name.
	*/
	const core::String& GetName() const { return m_Name; }

	//! The number of elements on this device
	/**
	\param type The event type given by the queried elements.
	\return The number of elments on the device.
	*/
	int GetElementCount(EDeviceEventType type) const
	{
		return m_Desc->GetElementCount(type);
	}

	//! The name of a device element.
	/**
	Intended for display.
	\param type The eventtype of the element.
	\param id The id of the element
	\return The name of the element.
	*/
	core::StringView GetElementName(EDeviceEventType type, int id) const
	{
		return m_Desc->GetElementName(type, id);
	}

	//! The exact type of the device element.
	/**
	\param type The eventtype of the element.
	\param id The id of the element
	\return A or'ed combination of element types.
	*/
	EDeviceElementType GetElementType(EDeviceEventType type, int id) const
	{
		return m_Desc->GetElementType(type, id);
	}

	//! Get the state of a button
	/**
	\param buttonCode The id of the button.
	*/
	bool GetButtonState(int buttonCode) const
	{
		return m_Buttons.At(buttonCode).state;
	}

	//! Get the state of an axis.
	/**
	\param axisCode The id of the axis.
	\return The value of the axis in device units.
	*/
	float GetAxisState(int axisCode) const
	{
		return m_Axes.At(axisCode).state;
	}

	//! Get the state of an area.
	/**
	\param areaCode The id of the area.
	\return The value of the area in device units.
	*/
	math::Vector2F GetAreaState(int areaCode) const
	{
		return m_Areas.At(areaCode).state;
	}

	//! Update the device with this event.
	/**
	Shouldn't be used directly by the user.
	To emulate input events use InputSystem::Update.
	\param [in] [out] event The event to change the device with,
	the device will fill missing information in the event(like absolute or relative movements).
	\return Was the state of the device changed by the event.
	*/
	LUX_API bool Update(Event& event);

private:
	struct ElementData
	{
		EDeviceElementType type;
	};

	struct ButtonElement : ElementData
	{
		bool state;
	};
	struct AxisElement : ElementData
	{
		float state;
	};
	struct AreaElement : ElementData
	{
		math::Vector2F state;
	};

private:
	StrongRef<InputDeviceDesc> m_Desc;
	core::Array<ButtonElement> m_Buttons;
	core::Array<AxisElement> m_Axes;
	core::Array<AreaElement> m_Areas;

	core::String m_Name;
	EDeviceType m_Type;
	StrongRef<InputSystem> m_System;
};

} // namespace input
} // namespace lux

#endif // #ifndef INCLUDED_LUX_INPUT_DEVICE_H