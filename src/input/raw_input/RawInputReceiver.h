#ifndef INCLUDED_RAW_INPUT_RECEIVER_H
#define INCLUDED_RAW_INPUT_RECEIVER_H
#include "input/InputSystem.h"
#include "core/lxHashMap.h"
#include "core/lxArray.h"
#include "core/lxMemory.h"

#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "input/raw_input/RawInputDevice.h"
#include "platform/StrippedWindows.h"

namespace lux
{
namespace input
{

enum class HIDUsagePage
{
	Undefined = 0x00,
	GenericDesktopControls = 0x01,
	SimulationControls = 0x02,
	VRControls = 0x03,
	SportControls = 0x04,
	GameControls = 0x05,
	GenericDeviceControls = 0x06,
	Keyboard = 0x07,
	LEDs = 0x08,
	Button = 0x09,
	Ordianal = 0x0A,
	Telephony = 0x0B,
	Consumer = 0x0C,
	Digitizer = 0x0D,
	//Reserved = 0x0E
	PIDPage = 0x0F,
	Unicode = 0x10,
	//Reserved = 0x11 - 0x13
	AlphanumericDisplay = 0x14,
	//Reserved = 0x15 - 0x3F
	MedicalInstruments = 0x40,
	//Reserved = 0x41 - 0x7F
	//MonitorPages = 0x80 - 0x83
	//PowerPages = 0x84 - 0x87
	//Reserved = 0x88 - 0x8B
	BarCodeScannerPage = 0x8C,
	MSRDevices = 0x8E,
	ReservedPointOfSalePages = 0x8F,
	CameraControlPage = 0x90,
};

class RawInputReceiver : public ReferenceCounted
{
public:
	RawInputReceiver(InputSystem* inputSystem, HWND window);
	~RawInputReceiver();

	void RegisterDevices(HIDUsagePage usagePage, s32 usageID, bool exclusive);
	void UnregisterDevice(HIDUsagePage usagePage, s32 usageID);
	void UnregisterAll();
	StrongRef<InputSystem> GetSystem() const;
	RAWINPUT* GetRawInputData(HRAWINPUT raw);
	StrongRef<RawInputDevice> CreateDevice(HANDLE rawHandle);
	void DestroyDevice(RawInputDevice* device);
	StrongRef<RawInputDevice> GetDevice(HANDLE rawHandle, u32 deviceHint=0);
	bool HandleMessage(UINT msg,
		WPARAM wParam,
		LPARAM lParam,
		LRESULT& result);

	u32 DiscoverDevices(EEventSource deviceType);

private:
	core::RawMemory m_RawData;
	HWND m_Window;
	StrongRef<InputSystem> m_InputSystem;

	core::HashMap<HANDLE, StrongRef<RawInputDevice>> m_DeviceMap;
	core::Array<RAWINPUTDEVICE> m_Subscribed;
};

}
}

#endif // LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_INPUT_RECEIVER_H