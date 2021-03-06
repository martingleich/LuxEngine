#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputReceiver.h"

#include "RawInputDeviceKeyboard.h"
#include "RawInputDeviceMouse.h"
#include "RawInputDeviceJoystick.h"

#include "platform/Win32Exception.h"

#include "core/Logger.h"

namespace lux
{
namespace input
{

RawInputReceiver::RawInputReceiver(InputSystem* inputSystem, HWND window) :
	m_Window(window),
	m_InputSystem(inputSystem)
{
	m_KeyboardLayout = GetKeyboardLayout(0);

	RegisterDevices(HIDUsagePage::GenericDesktopControls, 2, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 4, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 5, false);
	RegisterDevices(HIDUsagePage::GenericDesktopControls, 6, false);
}

RawInputReceiver::~RawInputReceiver()
{
	UnregisterAll();
}

void RawInputReceiver::RegisterDevices(HIDUsagePage usagePage, int usageID, bool exclusive)
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

void RawInputReceiver::UnregisterDevice(HIDUsagePage usagePage, int usageID)
{
	for(int i = 0; i < m_Subscribed.Size();) {
		auto& entry = m_Subscribed[i];
		if(entry.usUsagePage == (USHORT)usagePage && (entry.usUsage == usageID || usageID < 0)) {
			m_Subscribed.Erase(i);
			RAWINPUTDEVICE device;
			device.hwndTarget = m_Window;
			device.usUsage = (USHORT)usageID;
			device.usUsagePage = (USHORT)usagePage;
			device.dwFlags = RIDEV_REMOVE;
			RegisterRawInputDevices(&device, 1, sizeof(device));
		} else {
			++i;
		}
	}
	for(auto it = m_Subscribed.First(); it != m_Subscribed.End();) {
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
	UINT size = 0;
	::GetRawInputData(raw, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
	m_RawData.SetMinSize(size);

	UINT result = ::GetRawInputData(raw, RID_INPUT, m_RawData, &size, sizeof(RAWINPUTHEADER));
	if(result != size)
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
		return LUX_NEW(RawKeyboardDevice)(m_InputSystem, rawHandle, m_KeyboardLayout);
	case RIM_TYPEMOUSE:
		return LUX_NEW(RawMouseDevice)(m_InputSystem, rawHandle);
	case RIM_TYPEHID:
		return LUX_NEW(RawJoystickDevice)(m_InputSystem, rawHandle);
	default:
		throw core::GenericRuntimeException("Unsupported device type");
	}
}

void RawInputReceiver::DestroyDevice(RawInputDevice* device)
{
	for(auto it = m_DeviceMap.begin(); it != m_DeviceMap.end(); ++it) {
		if(it->value == device) {
			m_DeviceMap.EraseIter(it);
			break;
		}
	}
}

StrongRef<RawInputDevice> RawInputReceiver::GetDevice(HANDLE rawHandle, int deviceHint)
{
	StrongRef<RawInputDevice> out;
	if(!rawHandle) {
		// Seems to happen for some inputs, for example Mouse-Scrolling via Touchpad.
		// We look if there is a default device of the given type.
		EDeviceType deviceType;
		if(deviceHint == RIM_TYPEKEYBOARD)
			deviceType = EDeviceType::Keyboard;
		else if(deviceHint == RIM_TYPEMOUSE)
			deviceType = EDeviceType::Mouse;
		else
			return nullptr; // It's quite possible for many HID's to be active at the same time, so don't take risk

		for(auto& d : m_DeviceMap.Values()) {
			if(d->GetDescription()->GetType() == deviceType) {
				out = d;
				break;
			}
		}
	} else {
		auto itOpt = m_DeviceMap.Find(rawHandle);
		if(!itOpt.HasValue()) {
			out = CreateDevice(rawHandle);
			m_DeviceMap.SetAndReplace(rawHandle, out);
		} else {
			out = itOpt.GetValue()->value;
		}
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
				StrongRef<RawInputDevice> device = GetDevice(raw_data->header.hDevice, raw_data->header.dwType);

				if(device)
					device->HandleInput(raw_data);
			}
		} catch(core::RuntimeException&) {
			// Can't receive input -> just ignore the input
		}

		result = S_OK;
		// Since ret=false
		// Call DefWindowProc to clean up.
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

					if(userDevice) {
						/*
						TODO: Handle disconnect;
						*/
						(void)0;
					}

					DestroyDevice(device);
				}
			}
		} catch(core::RuntimeException& e) {
			log::Debug("Detected unsupported device({}): {}", hDevice, e.What().AsView());
		}
		result = S_OK;
		ret = true;
		break;
	}
	case WM_INPUTLANGCHANGE:
		m_KeyboardLayout = reinterpret_cast<HKL>(lParam);
		for(auto& device : m_DeviceMap.Values()) {
			if(device->GetDescription()->GetType() == EDeviceType::Keyboard) {
				auto keyboard = device.AsStrong<RawKeyboardDevice>();
				if(keyboard)
					keyboard->SetKeyboardLayout(m_KeyboardLayout);
			}
		}
		break;
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

int RawInputReceiver::DiscoverDevices(EDeviceType deviceType)
{
	DWORD win32DeviceType;
	if(deviceType == EDeviceType::Keyboard)
		win32DeviceType = RIM_TYPEKEYBOARD;
	else if(deviceType == EDeviceType::Mouse)
		win32DeviceType = RIM_TYPEMOUSE;
	else if(deviceType == EDeviceType::Joystick)
		win32DeviceType = RIM_TYPEHID;
	else
		return 0;

	core::Array<RAWINPUTDEVICELIST> device_list;
	UINT device_count;

	if(GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
		return 0;

	device_list.Resize(device_count);
	if(GetRawInputDeviceList(device_list.Data(), &device_count, sizeof(RAWINPUTDEVICELIST)) == -1)
		return 0;

	int count = 0;
	for(int i = 0; i < device_list.Size(); ++i) {
		RAWINPUTDEVICELIST& device_info = device_list[i];
		if(device_info.dwType == win32DeviceType) {
			try {
				// Force creation of device.
				StrongRef<RawInputDevice> device = GetDevice(device_info.hDevice);
				StrongRef<InputDevice> real_device = m_InputSystem->FindDevice(device->GetDescription());
				LUX_UNUSED(real_device);
				++count;
			} catch(core::RuntimeException& e) {
				log::Debug("Detected unsupported device({}): {}", device_info.hDevice, e.What().AsView());
				break;
			}
		}
	}

	return count;
}

}
}

#endif // LUX_COMPILE_WITH_RAW_INPUT
