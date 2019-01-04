#ifndef INCLUDED_LUX_RAW_INPUT_DEVICE_H
#define INCLUDED_LUX_RAW_INPUT_DEVICE_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/InputDevice.h"
#include "input/InputSystem.h"

#include "platform/StrippedWindows.h"

namespace lux
{
namespace input
{

class RawInputDeviceDescription : public InputDeviceDesc
{
public:
	struct ElemDesc
	{
		core::String name;
		EDeviceElementType type;

		ElemDesc(core::StringView _name, EDeviceElementType _type) :
			name(_name),
			type(_type)
		{
		}
	};

	EDeviceType GetType() const override { return type; }
	core::StringView GetName() const override { return name; }
	core::StringView GetGUID() const override { return guid; }

	int GetElementCount(EDeviceEventType eventType) const override
	{
		switch(eventType) {
		case EDeviceEventType::Button: return buttonCount;
		case EDeviceEventType::Axis: return axesCount;
		case EDeviceEventType::Area: return areasCount;
		}
		return 0;
	}
	core::StringView GetElementName(EDeviceEventType eventType, int id) const override
	{
		return desc.At(GetId(eventType, id)).name;
	}
	EDeviceElementType GetElementType(EDeviceEventType eventType, int id) const override
	{
		return desc.At(GetId(eventType, id)).type;
	}

	EDeviceType type;
	core::String name;
	core::String guid;
	int buttonCount;
	int axesCount;
	int areasCount;
	core::Array<ElemDesc> desc;

private:
	int GetId(EDeviceEventType eventType, int id) const
	{
		int off = 0;
		switch(eventType) {
		case EDeviceEventType::Button: off = 0; LX_CHECK_BOUNDS(id, 0, buttonCount); break;
		case EDeviceEventType::Axis: off = buttonCount; LX_CHECK_BOUNDS(id, 0, axesCount); break;
		case EDeviceEventType::Area: off = buttonCount + axesCount; LX_CHECK_BOUNDS(id, 0, areasCount); break;
		}
		return off+id;
	}
};

class RawInputDevice : public ReferenceCounted
{
public:
	RawInputDevice(InputSystem* system);

	virtual ~RawInputDevice() {}

	virtual void HandleInput(RAWINPUT* input) = 0;
	virtual StrongRef<InputDeviceDesc> GetDescription() = 0;

	StrongRef<InputDevice> GetDevice() const;

protected:
	void SendInputEvent(Event& event);

	static core::String GetDevicePath(HANDLE raw_handle);
	static core::String GetDeviceGUID(HANDLE raw_handle);
	static RID_DEVICE_INFO GetDeviceInfo(HANDLE raw_handle);

private:
	StrongRef<InputSystem> m_System;
	StrongRef<InputDevice> m_Device;
};

} // namespace input
} // namespace lux

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_LUX_RAW_INPUT_DEVICE_H