#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputReceiver.h"

#include "RawInputDeviceKeyboard.h"
#include "RawInputDeviceMouse.h"
#include "RawInputDeviceJoystick.h"

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
		m_Subscribed.Push_Back(device);
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

EResult RawInputReceiver::GetRawInputData(HRAWINPUT raw, RAWINPUT*& data)
{
	data = nullptr;
	UINT Size = 0;
	::GetRawInputData(raw, RID_INPUT, NULL, &Size, sizeof(RAWINPUTHEADER));
	m_RawData.SetMinSize(Size);

	UINT Return = ::GetRawInputData(raw, RID_INPUT, m_RawData, &Size, sizeof(RAWINPUTHEADER));
	if(Return != Size)
		return EResult::Failed;

	data = (RAWINPUT*)m_RawData;

	return EResult::Succeeded;
}

EResult RawInputReceiver::CreateDevice(HANDLE rawHandle, StrongRef<RawInputDevice>& device)
{
	device = nullptr;

	RID_DEVICE_INFO info;
	UINT buffer_size;
	info.cbSize = sizeof(info);
	buffer_size = sizeof(info);
	if(GetRawInputDeviceInfoW(rawHandle, RIDI_DEVICEINFO, &info, &buffer_size) == -1)
		return EResult::Failed;

	switch(info.dwType) {
	case RIM_TYPEKEYBOARD:
		device = LUX_NEW(RawKeyboardDevice)(m_InputSystem);
		break;
	case RIM_TYPEMOUSE:
		device = LUX_NEW(RawMouseDevice)(m_InputSystem);
		break;
	case RIM_TYPEHID:
		device = LUX_NEW(RawJoystickDevice)(m_InputSystem);
		break;
	default:
		lxAssertNeverReach("Unknown raw device type.");
		return EResult::Failed;
	}

	if(Failed(device->Init(rawHandle))) {
		device = nullptr;
		return EResult::Failed;
	}

	return EResult::Succeeded;
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

EResult RawInputReceiver::GetDevice(HANDLE rawHandle, StrongRef<RawInputDevice>& device)
{
	device = nullptr;

	auto it = m_DeviceMap.Find(rawHandle);
	if(it == m_DeviceMap.End()) {
		if(Failed(CreateDevice(rawHandle, device)))
			return EResult::Failed;

		m_DeviceMap.Set(rawHandle, device);
	} else {
		device = *it;
	}

	return EResult::Succeeded;
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
		RAWINPUT* raw_data;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), raw_data);
		if(raw_data) {
			StrongRef<RawInputDevice> device;
			GetDevice(raw_data->header.hDevice, device);

			if(device)
				device->HandleInput(raw_data);
		}
		result = S_OK;
		ret = true;
		break;
	}
	case WM_INPUT_DEVICE_CHANGE:
	{
		StrongRef<RawInputDevice> device;
		GetDevice(reinterpret_cast<HANDLE>(lParam), device);
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
			StrongRef<RawInputDevice> device;
			EResult r = GetDevice(device_info.hDevice, device);
			if(Succeeded(r)) {
				StrongRef<InputDevice> real_device = m_InputSystem->CreateDevice(device);
				real_device->Disconnect();
				++count;
			}
		}
	}

	return count;
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
