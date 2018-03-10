#ifndef INCLUDED_RAW_JOYSTICK_DEVICE_H
#define INCLUDED_RAW_JOYSTICK_DEVICE_H

#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT

#include "RawInputDevice.h"
#include "core/lxArray.h"
#include <hidsdi.h>

struct DIOBJECTATTRIBUTES
{
	DWORD dwFlags;
	WORD wUsagePage;
	WORD wUsage;
};

struct DIOBJECTCALIBRATION
{
	LONG lMin;
	LONG lCenter;
	LONG lMax;
};

namespace lux
{
namespace input
{

class RawJoystickDevice : public RawInputDevice
{
private:
	struct HIDObject
	{
		USAGE usagePage;
		USAGE usage;
		UCHAR reportID;

		USHORT index;

		core::String name;

		u32 code;

		bool isAbsolute;
	};

	struct Axis : HIDObject
	{
		int32_t logicalMin;
		int32_t logicalMax;

		bool isCalibrated;
		int32_t logicalCalibratedMin;
		int32_t logicalCalibratedMax;
		int32_t logicalCalibratedCenter;
	};

	struct Button : HIDObject
	{};

	struct Mapping
	{
		WORD usagePage;
		WORD usage;
		wchar_t name[32];
	};

	struct MappingAndCalibration
	{
		WORD usagePage;
		WORD usage;
		wchar_t name[32];

		bool isCalibrated;
		DIOBJECTCALIBRATION calibration;
	};

public:
	RawJoystickDevice(InputSystem* system, HANDLE rawHandle);
	~RawJoystickDevice();
	HANDLE GetDeviceHandle();
	core::String GetDeviceName();
	void GetButtonCaps(const HIDP_CAPS& deviceCaps, core::Array<HIDP_BUTTON_CAPS>& buttonCaps, size_t& buttonCount);
	void GetAxesCaps(const HIDP_CAPS& deviceCaps, core::Array<HIDP_VALUE_CAPS>& valueCaps, size_t& valueCount);
	void LoadDirectInputMapping(bool isAxis, Mapping* mappings, size_t mappingCount, size_t offset, const HIDD_ATTRIBUTES& attribs);
	void LoadDirectInputAxisCalibration(MappingAndCalibration* calibrationMapping, size_t mappingCount, const HIDD_ATTRIBUTES& attribs);
	void HandleInput(RAWINPUT* input);
	EEventSource GetType() const;
	size_t GetElementCount(EEventType type) const;
	ElemDesc GetElementDesc(EEventType type, u32 code) const;

private:
	HANDLE m_RawInputHandle;
	HANDLE m_NtHandle;

	size_t m_ReportSize;

	PHIDP_PREPARSED_DATA m_InputReportProtocol;

	core::Array<Axis> m_Axes;
	core::Array<Button> m_Buttons;

	core::Array<int> m_CodeHIDMapping;

	core::Array<bool> m_ButtonStates;
	core::Array<bool> m_NewButtonStates;
};

}
}

#endif // !LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_RAW_JOYSTICK_DEVICE_H