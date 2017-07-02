#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputReceiver.h"

#include "RawInputDeviceKeyboard.h"
#include "RawInputDeviceMouse.h"
#include "RawInputDeviceJoystick.h"

#include "Win32Exception.h"

#include "core/Logger.h"

namespace lux
{
namespace input
{

RawInputReceiver::RawInputReceiver(InputSystem* inputSystem, HWND window) :
	m_Window(window),
	m_InputSystem(inputSystem)
{
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 2, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 4, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 5, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 6, false);
}

RawInputReceiver::~RawInputReceiver()
{
	UnregisterAll();
}

void RawInputReceiver::RegisterDevices(HIDUsagePage usagePage, s32 usageID, bool exclusive)
{
	LUX_UNUSED(exclusive);

	RAWINPUTDEVICE device;
	device.hwndTarget = m_Window;
	device.usUsage = (USHORT)usageID;
	device.usUsagePage = (USHORT)usagePage;
	device.dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	if(RegisterRawInputDevices(&device, 1, sizeof(device)))
		m_Subscribed.PushBack(device);
}

void RawInputReceiver::UnregisterDevice(HIDUsagePage usagePage, s32 usageID)
{
	for(auto it = m_Subscribed.First(); it != m_Subscribed.End();) {
		if(it->usUsagePage == (int)usagePage && (it->usUsage == usageID || usageID < 0)) {
			m_Subscribed.Erase(it++);
			RAWINPUTDEVICE device;
			device.hwndTarget = m_Window;
			device.usUsage = (USHORT)usageID;
			device.usUsagePage = (USHORT)usagePage;
			device.dwFlags = RIDEV_REMOVE;
			RegisterRawInputDevices(&device, 1, sizeof(device));
		} else {
			++it;
		}
	}
}

void RawInputReceiver::UnregisterAll()
{
	for(auto it = m_Subscribed.First(); it != m_Subscribed.End(); ++it) {
		RAWINPUTDEVICE device;
		device.hwndTarget = m_Window;
		device.usUsage = it->usUsage;
		device.usUsagePage = it->usUsagePage;
		device.dwFlags = RIDEV_REMOVE;
		RegisterRawInputDevices(&device, 1, sizeof(device));
	}

	m_DeviceMap.Clear();
	m_Subscribed.Clear();
}

StrongRef<InputSystem> RawInputReceiver::GetSystem() const
{
	return m_InputSystem;
}

RAWINPUT* RawInputReceiver::GetRawInputData(HRAWINPUT raw)
{
	UINT Size = 0;
	::GetRawInputData(raw, RID_INPUT, NULL, &Size, sizeof(RAWINPUTHEADER));
	m_RawData.SetMinSize(Size);

	UINT Return = ::GetRawInputData(raw, RID_INPUT, m_RawData, &Size, sizeof(RAWINPUTHEADER));
	if(Return != Size)
		throw core::Win32Exception(GetLastError());

	return (RAWINPUT*)m_RawData;
}

StrongRef<RawInputDevice> RawInputReceiver::CreateDevice(HANDLE rawHandle)
{
	RID_DEVICE_INFO info;
	UINT buffer_size;
	info.cbSize = sizeof(info);
	buffer_size = sizeof(info);
	if(GetRawInputDeviceInfoW(rawHandle, RIDI_DEVICEINFO, &info, &buffer_size) == -1)
		throw core::Win32Exception(GetLastError());

	switch(info.dwType) {
	case RIM_TYPEKEYBOARD:
		return LUX_NEW(RawKeyboardDevice)(m_InputSystem, rawHandle);
		break;
	case RIM_TYPEMOUSE:
		return LUX_NEW(RawMouseDevice)(m_InputSystem, rawHandle);
		break;
	case RIM_TYPEHID:
		return LUX_NEW(RawJoystickDevice)(m_InputSystem, rawHandle);
		break;
	default:
		throw core::RuntimeException("Unsupported device type");
	}
}

void RawInputReceiver::DestroyDevice(RawInputDevice* device)
{
	for(auto it = m_DeviceMap.First(); it != m_DeviceMap.End(); ++it) {
		if(*it == device) {
			m_DeviceMap.Erase(it);
			break;
		}
	}
}

StrongRef<RawInputDevice> RawInputReceiver::GetDevice(HANDLE rawHandle)
{
	auto it = m_DeviceMap.Find(rawHandle);
	StrongRef<RawInputDevice> out;
	if(it == m_DeviceMap.End()) {
		out = CreateDevice(rawHandle);
		m_DeviceMap.Set(rawHandle, out);
	} else {
		out = *it;
	}

	return out;
}

bool RawInputReceiver::HandleMessage(UINT msg,
	WPARAM wParam,
	LPARAM lParam,
	LRESULT& result)
{
	bool ret = false;
	switch(msg) {
	case WM_INPUT:
	{
		try {
			RAWINPUT* raw_data = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam));
			if(raw_data) {
				StrongRef<RawInputDevice> device = GetDevice(raw_data->header.hDevice);

				if(device)
					device->HandleInput(raw_data);
			}
		} catch(core::RuntimeException&) {
			// Can't receive input -> just ignore the input
		}
		result = S_OK;
		ret = true;
		break;
	}
	case WM_INPUT_DEVICE_CHANGE:
	{
		HANDLE hDevice = reinterpret_cast<HANDLE>(lParam);
		try {
			StrongRef<RawInputDevice> device = GetDevice(hDevice);
			if(device) {
				if(wParam == GIDC_ARRIVAL)
					(void)0; /* We only connect with devices, which have sent input at least one time*/
				if(wParam == GIDC_REMOVAL) {
					InputDevice* userDevice = device->GetDevice();

					if(userDevice)
						userDevice->Disconnect();

					DestroyDevice(device);
				}
			}
		} catch(core::RuntimeException& e) {
			log::Debug("Detected unsupported device(~a): ~s", hDevice, e.What());
		}
		result = S_OK;
		ret = true;
		break;
	}
	case WM_SETFOCUS:
		if(m_InputSystem)
			m_InputSystem->SetForegroundState(true);
		break;

	case WM_KILLFOCUS:
		if(m_InputSystem)
			m_InputSystem->SetForegroundState(false);
		break;
	}

	return ret;
}

u32 RawInputReceiver::DiscoverDevices(EEventSource deviceType)
{
	DWORD win32DeviceType;
	if(deviceType == EEventSource::Keyboard)
		win32DeviceType = RIM_TYPEKEYBOARD;
	else if(deviceType == EEventSource::Mouse)
		win32DeviceType = RIM_TYPEMOUSE;
	else if(deviceType == EEventSource::Joystick)
		win32DeviceType = RIM_TYPEHID;
	else
		return 0;

	core::array<RAWINPUTDEVICELIST> device_list;
	UINT device_count;

	if(GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
		return 0;

	device_list.Resize(device_count);
	if(GetRawInputDeviceList(device_list.Data(), &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
		return 0;

	u32 count = 0;
	for(size_t i = 0; i < device_list.Size(); ++i) {
		RAWINPUTDEVICELIST& device_info = device_list[i];
		if(device_info.dwType == win32DeviceType) {
			try {
				StrongRef<RawInputDevice> device = GetDevice(device_info.hDevice);
				StrongRef<InputDevice> real_device = m_InputSystem->CreateDevice(device);
				real_device->Disconnect();
				++count;
			} catch(core::RuntimeException& e) {
				log::Debug("Detected unsupported device(~a): ~s", device_info.hDevice, e.What());
				break;
			}
		}
	}

	return count;
}

}
}

#endif // LUX_COMPILE_WITH_RAW_INPUT
