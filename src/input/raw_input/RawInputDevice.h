#ifndef INCLUDED_RAW_INPUT_DEVICE_H
#define INCLUDED_RAW_INPUT_DEVICE_H
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/InputDevice.h"
#include "input/InputSystem.h"

#include "platform/StrippedWindows.h"

namespace lux
{
namespace input
{

class RawInputDevice : public DeviceCreationDesc
{
public:
	RawInputDevice(InputSystem* system);

	virtual ~RawInputDevice() {}

	virtual void HandleInput(RAWINPUT* input) = 0;

	StrongRef<InputDevice> GetDevice() const;
	const String& GetName() const;
	const String& GetGUID() const;

protected:
	void SendInputEvent(Event& event);
	static String GetDevicePath(HANDLE raw_handle);
	static String GetDeviceGUID(HANDLE raw_handle);
	static RID_DEVICE_INFO GetDeviceInfo(HANDLE raw_handle);

protected:
	String m_Name;
	String m_GUID;

private:
	StrongRef<InputSystem> m_System;
	StrongRef<InputDevice> m_Device;
};

} // namespace input
} // namespace lux

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_INPUT_DEVICE_H