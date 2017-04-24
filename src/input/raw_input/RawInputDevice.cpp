#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDevice.h"
#include "core/lxUnicodeConversion.h"

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

const string& RawInputDevice::GetName() const
{
	return m_Name;
}

const string& RawInputDevice::GetGUID() const
{
	return m_GUID;
}

void RawInputDevice::SendInputEvent(Event& event)
{
	if(!m_Device)
		m_Device = m_System->CreateDevice(this);

	event.device = m_Device;
	event.source = m_Device->GetType();

	event.shift = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
	event.control = ((GetKeyState(VK_CONTROL) & 0x8000) != 0);

	m_System->Update(event);
}

EResult RawInputDevice::GetDevicePath(HANDLE raw_handle, string& path)
{
	// Get Buffer Size
	UINT bufferSize = 0;
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICENAME, NULL, &bufferSize) != 0)
		return EResult::Failed;

	// Get Buffer Data
	core::array<u16> str;
	str.Resize(bufferSize);
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICENAME, str.Data(), &bufferSize) == -1)
		return EResult::Failed;

	path = core::UTF16ToString(str.Data_c());
	path.ReplaceRange("\\", path.First() + 1, 1); // Old windows bug, sometimes the second character is not a backslash.

	return EResult::Succeeded;
}

EResult RawInputDevice::GetDeviceGUID(HANDLE raw_handle, string& guid)
{
	guid.Clear();

	string path;
	if(Failed(GetDevicePath(raw_handle, path)))
		return EResult::Failed;

	// A guid is build like: random_characters{<guid>}
	for(auto it = path.First(); it != path.End(); ++it) {
		if(*it == '{') {
			guid = path.SubString(it.Next(), path.Last());
			break;
		}
	}

	return EResult::Succeeded;
}

EResult RawInputDevice::GetDeviceInfo(HANDLE raw_handle, RID_DEVICE_INFO& info)
{
	UINT bufferSize = sizeof(info);
	if(GetRawInputDeviceInfoW(raw_handle, RIDI_DEVICEINFO, &info, &bufferSize))
		return EResult::Failed;

	return EResult::Succeeded;
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
