#ifndef INCLUDED_LUX_RAW_JOYSTICK_DEVICE_H
#define INCLUDED_LUX_RAW_JOYSTICK_DEVICE_H

#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT

#include "RawInputDevice.h"
#include "core/lxArray.h"
#include <hidsdi.h>
#include "platform/WindowsUtils.h"

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
		WORD usagePage = 0;
		WORD usage = 0;
		wchar_t name[32];
	};

	struct MappingAndCalibration : Mapping
	{
		bool isCalibrated;
		DIOBJECTCALIBRATION calibration;
	};

public:
	RawJoystickDevice(InputSystem* system, HANDLE rawHandle);
	~RawJoystickDevice();

	StrongRef<InputDeviceDesc> GetDescription() override { return m_Desc; }
	void HandleInput(RAWINPUT* input);

private:
	void CreateAxes(const HIDP_CAPS& caps, const HIDD_ATTRIBUTES* deviceAttributes);
	void CreateButtons(const HIDP_CAPS& caps, const HIDD_ATTRIBUTES* deviceAttributes);
	static Win32FileHandle GetDeviceHandle(HANDLE rawHandle);
	static core::String GetDeviceName(const Win32FileHandle& fileHandle);

	struct ButtonCaps
	{
		CHAR reportId;
		bool isAbsolute;
		USAGE usagePage;
		USAGE usage;
		USHORT dataIndex;
	};
	struct AxisCaps
	{
		CHAR reportId;
		bool isAbsolute;
		USAGE usagePage;
		USAGE usage;
		USHORT dataIndex;

		LONG logicalMin;
		LONG logicalMax;
	};

	static core::Array<ButtonCaps> GetButtonCaps(const HIDP_CAPS& deviceCaps, PHIDP_PREPARSED_DATA inputReportProtocol);
	static core::Array<AxisCaps> GetAxesCaps(const HIDP_CAPS& deviceCaps, PHIDP_PREPARSED_DATA inputReportProtocol);

	static void LoadDirectInputMapping(bool isAxis, Mapping* mappings, int mappingCount, int offset, const HIDD_ATTRIBUTES& attribs);
	static void LoadDirectInputAxisCalibration(MappingAndCalibration* calibrationMapping, int mappingCount, const HIDD_ATTRIBUTES& attribs);

	void InitDeviceDescription();

	static void FreePreparsedData(PHIDP_PREPARSED_DATA ptr)
	{
		HidD_FreePreparsedData(ptr);
	}
private:
	HANDLE m_RawInputHandle;
	Win32FileHandle m_NtHandle;

	PointerWrapper<PHIDP_PREPARSED_DATA, FreePreparsedData> m_InputReportProtocol;

	core::Array<Axis> m_Axes;
	core::Array<Button> m_Buttons;

	core::Array<int> m_CodeHIDMapping;

	core::Array<bool> m_ButtonStates;
	core::Array<bool> m_NewButtonStates;

	StrongRef<RawInputDeviceDescription> m_Desc;
};

}
}

#endif // !LUX_COMPILE_WITH_RAW_INPUT

#endif // !INCLUDED_LUX_RAW_JOYSTICK_DEVICE_H