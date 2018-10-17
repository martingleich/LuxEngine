#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDevice.h"
#include "core/lxUnicodeConversion.h"
#include "platform/Win32Exception.h"

namespace lux
{
namespace input
{

RawInputDevice::RawInputDevice(InputSystem* system) :
	m_System(system)
{
}

StrongRef<InputDevice> RawInputDevice::GetDevice() const
{
	return m_Device;
}

const core::String& RawInputDevice::GetName() const
{
	return m_Name;
}

const core::String& RawInputDevice::GetGUID() const
{
	return m_GUID;
}

void RawInputDevice::SendInputEvent(Event& event)
{
	if(!m_Device)
		m_Device = m_System->CreateDevice(this);

	event.device = m_Device;
	event.source = m_Device->GetType();

	event.shift = (GetKeyState(VK_SHIFT)&0x8000) != 0;
	event.control = (GetKeyState(VK_CONTROL)&0x8000) != 0;

	m_System->Update(event);
}

core::String RawInputDevice::GetDevicePath(HANDLE raw_handle)
{
	// Get Buffer Size
	UINT bufferSize = 0;
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICENAME, NULL, &bufferSize) != 0)
		throw core::Win32Exception(GetLastError());

	// Get Buffer Data
	core::Array<u16> str;
	str.Resize(bufferSize);
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICENAME, str.Data(), &bufferSize) == -1)
		throw core::Win32Exception(GetLastError());

	core::String path = core::UTF16ToString(str.Data(), -1);
	path.ReplaceRange("\\", 1, 1); // Old windows bug, sometimes the second character is not a backslash.

	return path;
}

core::String RawInputDevice::GetDeviceGUID(HANDLE raw_handle)
{
	core::String path = GetDevicePath(raw_handle);

	// A guid is build like: random_characters{<guid>}
	int i = path.Find("{");
	if(i >= 0)
		return path.EndSubString(i+1);
	throw core::GenericRuntimeException("Invalid device guid");
}

RID_DEVICE_INFO RawInputDevice::GetDeviceInfo(HANDLE raw_handle)
{
	RID_DEVICE_INFO out;
	UINT bufferSize = sizeof(out);
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICEINFO, &out, &bufferSize) == 0)
		throw core::Win32Exception(GetLastError());

	return out;
}

} // namespace input
} // namespace lux

#endif // LUX_COMPILE_WITH_RAW_INPUT
