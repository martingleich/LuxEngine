#ifndef INCLUDED_RAW_INPUT_DEVICE_H
#define INCLUDED_RAW_INPUT_DEVICE_H
#include "input/InputDevice.h"
#include "input/InputSystem.h"

#ifdef LUX_COMPILE_WITH_RAW_INPUT
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

	virtual EResult Init(HANDLE rawHandle) = 0;
	virtual EResult HandleInput(RAWINPUT* input) = 0;

	StrongRef<InputDevice> GetDevice() const;
	const string& GetName() const;
	const string& GetGUID() const;

protected:
	void SendInputEvent(Event& event);
	static EResult GetDevicePath(HANDLE raw_handle, string& path);
	static EResult GetDeviceGUID(HANDLE raw_handle, string& guid);
	static EResult GetDeviceInfo(HANDLE raw_handle, RID_DEVICE_INFO& info);

protected:
	string m_Name;
	string m_GUID;

private:
	StrongRef<InputSystem> m_System;
	StrongRef<InputDevice> m_Device;
};

}
}

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_INPUT_DEVICE_H