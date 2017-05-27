#ifndef INCLUDED_RAW_INPUT_DEVICE_H
#define INCLUDED_RAW_INPUT_DEVICE_H
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/InputDevice.h"
#include "input/InputSystem.h"

#include "StrippedWindows.h"

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
	const string& GetName() const;
	const string& GetGUID() const;

protected:
	void SendInputEvent(Event& event);
	static string GetDevicePath(HANDLE raw_handle);
	static string GetDeviceGUID(HANDLE raw_handle);
	static RID_DEVICE_INFO GetDeviceInfo(HANDLE raw_handle);

protected:
	string m_Name;
	string m_GUID;

private:
	StrongRef<InputSystem> m_System;
	StrongRef<InputDevice> m_Device;
};

} // namespace input
} // namespace lux

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_INPUT_DEVICE_H