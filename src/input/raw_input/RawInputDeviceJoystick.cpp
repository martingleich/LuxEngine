#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceJoystick.h"

#include "core/lxAlgorithm.h"
#include "core/StringConverter.h"

#include "core/lxUnicodeConversion.h"
#include "platform/Win32Exception.h"

namespace lux
{
namespace input
{

/*
Cleanup this whole class it seem quite instable.
Remove the m_CodeHIDMapping array
	and just place the elements in the correct order.
Cleanup mapping of buttons to names and calibrations.
	Use a HashMap to get the mapping instead of some random sized array.
	Get the existing elements in the registry in a smarter way.
Change buttonCaps AxisCaps to only contain single elements.
*/
RawJoystickDevice::RawJoystickDevice(InputSystem* system, HANDLE rawHandle) :
	RawInputDevice(system),
	m_RawInputHandle(rawHandle)
{
	log::Warning("RawJoystickDevice is still in work and might be unstable.");

	m_NtHandle = GetDeviceHandle(rawHandle);

	if(!HidD_GetPreparsedData(m_NtHandle, m_InputReportProtocol.Access()))
		throw core::Win32Exception(GetLastError());

	HIDP_CAPS caps;
	if(HIDP_STATUS_SUCCESS != HidP_GetCaps(m_InputReportProtocol, &caps))
		throw core::Win32Exception(GetLastError());

	HIDD_ATTRIBUTES deviceAttributes;
	BOOL hasDeviceAttributes = HidD_GetAttributes(m_NtHandle, &deviceAttributes);
	auto deviceAttributesPointer = hasDeviceAttributes ? &deviceAttributes : nullptr;

	CreateButtons(caps, deviceAttributesPointer);
	CreateAxes(caps, deviceAttributesPointer);

	struct HIDObjectWrapper
	{
		HIDObject* obj;
		int id;
	};

	struct SortByUsage
	{
		bool Smaller(const HIDObjectWrapper& a, const HIDObjectWrapper& b) const
		{
			if(a.obj->usagePage >= b.obj->usagePage)
				return false;
			if(a.obj->usage >= b.obj->usage)
				return false;
			if(a.obj->reportID >= b.obj->reportID)
				return false;

			return true;
		}
	};

	struct SortByIndex
	{
		bool Smaller(const HIDObject& a, const HIDObject& b) const
		{
			return a.index < b.index;
		}
	};

	m_Buttons.Sort(SortByIndex());

	m_CodeHIDMapping.Resize(m_Buttons.Size() + m_Axes.Size());

	core::Array<HIDObjectWrapper> temp;
	int index = 0;
	for(auto it = m_Buttons.First(); it != m_Buttons.End(); ++it) {
		HIDObjectWrapper wrapper;
		wrapper.obj = &*it;
		wrapper.id = index;
		temp.PushBack(wrapper);
		++index;
	}

	temp.Sort(SortByUsage());
	int code = 0;
	for(auto it = temp.First(); it != temp.End(); ++it) {
		it->obj->code = code;

		m_CodeHIDMapping[code] = it->id;
		++code;
	}

	temp.Clear();
	index = 0;
	for(auto it = m_Axes.First(); it != m_Axes.End(); ++it) {
		HIDObjectWrapper wrapper;
		wrapper.obj = &*it;
		wrapper.id = index;
		temp.PushBack(wrapper);
		++index;
	}

	temp.Sort(SortByUsage());
	code = 0;
	for(auto it = temp.First(); it != temp.End(); ++it) {
		it->obj->code = code;

		m_CodeHIDMapping[m_Buttons.Size() + code] = it->id;
		++code;
	}

	// Create buttons states to detect changes.
	m_ButtonStates.Resize(m_Buttons.Size(), false);
	m_NewButtonStates.Resize(m_Buttons.Size(), false);

	InitDeviceDescription();
}

void RawJoystickDevice::CreateAxes(const HIDP_CAPS &caps, const HIDD_ATTRIBUTES* deviceAttributes)
{
	// Direct input only supporte 7 axes.
	MappingAndCalibration directInputAxisMapping[7] = {};
	auto axesCaps = GetAxesCaps(caps, m_InputReportProtocol);

	// Default initilize mappings.
	for(auto it = axesCaps.First(); it != axesCaps.End(); ++it) {
		int index = it->usage - HID_USAGE_GENERIC_X;
		if(index >= 0 && index < 7) {
			directInputAxisMapping[index].usagePage = it->usagePage;
			directInputAxisMapping[index].usage = it->usage;
		}

		if(directInputAxisMapping[2].usagePage == 0) {
			// Remap Slider to Z if Z is unused
			// Ref: msdn.microsoft.com/en-us/library/windows/hardware/ff543445
			directInputAxisMapping[2] = directInputAxisMapping[6];
			// Invalidate old slider
			directInputAxisMapping[6].usage = 0;
			directInputAxisMapping[6].usagePage = 0;
		}
	}

	if(deviceAttributes) {
		LoadDirectInputMapping(true, directInputAxisMapping, 7, sizeof(directInputAxisMapping[0]), *deviceAttributes);
		LoadDirectInputAxisCalibration(directInputAxisMapping, 7, *deviceAttributes);
	}

	for(auto it = axesCaps.First(); it != axesCaps.End(); ++it) {
		bool isCalibrated = false;
		int32_t calibratedMin = 0;
		int32_t calibratedMax = 0;
		int32_t calibratedCenter = 0;
		wchar_t const *toName = L"";

		for(int i = 0; i < ARRAYSIZE(directInputAxisMapping); ++i) {
			auto& mapping = directInputAxisMapping[i];

			if(it->usagePage == mapping.usagePage && it->usage == mapping.usage) {
				toName = mapping.name;
				isCalibrated = mapping.isCalibrated;
				if(isCalibrated) {
					calibratedMin = mapping.calibration.lMin;
					calibratedMax = mapping.calibration.lMax;
					calibratedCenter = mapping.calibration.lCenter;
				}

				mapping.usage = 0;
				break;
			}
		}

		Axis axis;
		axis.usagePage = it->usagePage;
		axis.usage = it->usage;
		axis.index = it->dataIndex;
		axis.reportID = it->reportId;
		axis.logicalMin = it->logicalMin;
		axis.logicalMax = it->logicalMax;
		axis.isCalibrated = isCalibrated;
		axis.isAbsolute = it->isAbsolute;
		if(axis.isCalibrated) {
			axis.logicalCalibratedMin = calibratedMin;
			axis.logicalCalibratedMax = calibratedMax;
			axis.logicalCalibratedCenter = calibratedCenter;
		}

		axis.name = core::UTF16ToString(toName, -1);
		m_Axes.PushBack(axis);
	}
}

void RawJoystickDevice::CreateButtons(const HIDP_CAPS& caps, const HIDD_ATTRIBUTES* deviceAttributes)
{
	// Direct input only supporte 128 buttons.
	Mapping directInputButtonMapping[128];
	auto buttonCaps = GetButtonCaps(caps, m_InputReportProtocol);

	// Default initialize directInputButtonMapping
	for(auto& b : buttonCaps) {
		directInputButtonMapping[b.usage - 1].usagePage = b.usagePage;
		directInputButtonMapping[b.usage - 1].usage = b.usage;
		directInputButtonMapping[b.usage - 1].name[0] = 0;
	}

	// Load configuration from registry, if available.
	// And overwrite default values.
	if(deviceAttributes)
		LoadDirectInputMapping(false, directInputButtonMapping, 128, sizeof(directInputButtonMapping[0]), *deviceAttributes);

	// Create button objects from description and mapping.
	for(auto& b : buttonCaps) {
		wchar_t const* toName = L"";
		// Find the corresponding mapping, and get it's name.
		for(int i = 0; i < 128; ++i) {
			auto& mapping = directInputButtonMapping[i];

			if(b.usagePage == mapping.usagePage && b.usage == mapping.usage) {
				toName = mapping.name;
				break;
			}
		}

		// Create the new button.
		Button button;
		button.usagePage = b.usagePage;
		button.usage = b.usage;
		button.reportID = b.reportId;
		button.index = b.dataIndex;
		button.name = core::UTF16ToString(toName, -1);
		button.isAbsolute = b.isAbsolute;

		m_Buttons.PushBack(button);
	}
}

RawJoystickDevice::~RawJoystickDevice()
{
}

Win32FileHandle RawJoystickDevice::GetDeviceHandle(HANDLE rawHandle)
{
	core::String path = GetDevicePath(rawHandle);

	Win32FileHandle ntHandle = CreateFileW(core::UTF8ToWin32String(path), 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0, NULL);

	if(ntHandle == INVALID_HANDLE_VALUE)
		throw core::Win32Exception(GetLastError());

	return ntHandle;
}

core::String RawJoystickDevice::GetDeviceName(const Win32FileHandle& fileHandle)
{
	const int max_size = 127;

	wchar_t nameBuffer[max_size];

	int len = 0;
	if(HidD_GetManufacturerString(fileHandle, nameBuffer, max_size * 2)) {
		len = (int)wcslen(nameBuffer);
		nameBuffer[len++] = ' ';
	}

	if(HidD_GetProductString(fileHandle, nameBuffer + len, ULONG(max_size - len) * 2)) {
		len = (int)wcslen(nameBuffer);
	}

	if(len == 0)
		return "(unknown)";
	else
		return core::UTF16ToString(nameBuffer, len * 2);
}

core::Array<RawJoystickDevice::ButtonCaps> RawJoystickDevice::GetButtonCaps(
	const HIDP_CAPS& deviceCaps, PHIDP_PREPARSED_DATA inputReportProtocol)
{
	USHORT numberInputButtonCaps = deviceCaps.NumberInputButtonCaps;
	core::Array<HIDP_BUTTON_CAPS> caps;
	caps.Resize(deviceCaps.NumberInputButtonCaps);
	auto result = HidP_GetButtonCaps(HidP_Input, caps.Data(), &numberInputButtonCaps, inputReportProtocol);
	if(result != HIDP_STATUS_SUCCESS)
		throw core::Win32Exception(GetLastError());
	caps.Resize(numberInputButtonCaps);

	core::Array<ButtonCaps> out;
	for(auto& c : caps) {
		if(c.UsagePage != HID_USAGE_PAGE_BUTTON)
			continue;

		ButtonCaps bc;
		bc.reportId = c.ReportID;
		bc.usagePage = c.UsagePage;
		bc.isAbsolute = c.IsAbsolute;
		if(c.IsRange) {
			USAGE curUsage = c.Range.UsageMin;
			USHORT curDataIndex = c.Range.DataIndexMin;
			for(; curUsage < c.Range.UsageMax; ++curUsage, ++curDataIndex) {
				bc.usage = curUsage;
				bc.dataIndex = curDataIndex;
				out.PushBack(bc);
			}
		} else {
			bc.usage = c.NotRange.Usage;
			bc.dataIndex = c.NotRange.DataIndex;
		}
	}

	return out;
}

core::Array<RawJoystickDevice::AxisCaps> RawJoystickDevice::GetAxesCaps(
	const HIDP_CAPS& deviceCaps, PHIDP_PREPARSED_DATA inputReportProtocol)
{
	core::Array<HIDP_VALUE_CAPS> valueCaps;
	USHORT numberInputValueCaps = deviceCaps.NumberInputValueCaps;
	valueCaps.Resize(numberInputValueCaps);
	auto result = HidP_GetValueCaps(HidP_Input, valueCaps.Data(), &numberInputValueCaps, inputReportProtocol);
	valueCaps.Resize(numberInputValueCaps);
	if(result != HIDP_STATUS_SUCCESS)
		throw core::Win32Exception(GetLastError());

	core::Array<AxisCaps> out;
	for(auto& c : valueCaps) {
		if(c.UsagePage != HID_USAGE_PAGE_GENERIC)
			break;

		AxisCaps ac;
		ac.reportId = c.ReportID;
		ac.usagePage = c.UsagePage;
		ac.isAbsolute = c.IsAbsolute;

		if(c.LogicalMax < c.LogicalMin) {
			ac.logicalMin = c.LogicalMin > 0 ? c.LogicalMin : 0;
			ac.logicalMax = (1 << c.BitSize);
		}
		if((1 << c.BitSize) < c.LogicalMax)
			ac.logicalMax = (1 << c.BitSize);

		if(c.IsRange) {
			USAGE curUsage = c.Range.UsageMin;
			USHORT curDataIndex = c.Range.DataIndexMin;
			for(; curUsage < c.Range.UsageMax; ++curUsage, ++curDataIndex) {
				ac.usage = curUsage;
				ac.dataIndex = curDataIndex;
				out.PushBack(ac);
			}
		} else {
			ac.usage = c.NotRange.Usage;
			ac.dataIndex = c.NotRange.DataIndex;
			out.PushBack(ac);
		}
	}

	return out;
}

void RawJoystickDevice::LoadDirectInputMapping(bool isAxis, Mapping* mappings, int mappingCount, int offset, const HIDD_ATTRIBUTES& attribs)
{
	wchar_t* subType = isAxis ? L"Axes" : L"Buttons";

	wchar_t path[128];
	wsprintfW(path, L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%04X&PID_%04X\\%s\\", attribs.VendorID, attribs.ProductID, subType);

	int numberBegin = (int)wcslen(path);
	for(int i = 0; i < mappingCount; ++i) {
		wsprintfW(path + numberBegin, L"%d", i);

		HKEY key = NULL;
		if(0 != RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_READ, &key))
			continue;

		DWORD valueType;
		DWORD valueSize;

		Mapping& curMap = *(Mapping*)((char*)mappings + i * offset);

		valueType = REG_NONE;
		valueSize = 0;
		RegQueryValueExW(key, L"", NULL, &valueType, NULL, &valueSize);
		if(valueType == REG_SZ && sizeof(curMap.name) > valueSize) {
			RegQueryValueExW(key, L"", NULL, &valueType, (LPBYTE)curMap.name, &valueSize);
			curMap.name[valueSize] = 0;
		}

		DIOBJECTATTRIBUTES mapping;
		valueType = REG_NONE;
		valueSize = 0;
		RegQueryValueExW(key, L"Attributes", NULL, &valueType, NULL, &valueSize);
		if(valueType == REG_BINARY && valueSize == sizeof(mapping)) {
			RegQueryValueExW(key, L"Attributes", NULL, &valueType, (LPBYTE)&mapping, &valueSize);
			curMap.usagePage = mapping.wUsagePage;
			curMap.usage = mapping.wUsage;
		}

		RegCloseKey(key);
	}
}

void RawJoystickDevice::LoadDirectInputAxisCalibration(MappingAndCalibration* calibrationMapping, int mappingCount, const HIDD_ATTRIBUTES& attribs)
{
	wchar_t path[128];
	wsprintfW(path, L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_%04X&PID_%04X\\Calibration\\0\\Type\\Axes\\", attribs.VendorID, attribs.ProductID);

	int numberBegin = (int)wcslen(path);
	for(int i = 0; i < mappingCount; ++i) {
		wsprintfW(path + numberBegin, L"%d", i);

		HKEY key = NULL;
		if(0 == RegOpenKeyExW(HKEY_CURRENT_USER, path, 0u, KEY_READ, &key)) {
			DIOBJECTCALIBRATION& calibration = calibrationMapping[i].calibration;

			DWORD valueType = REG_NONE;
			DWORD valueSize = 0;
			RegQueryValueExW(key, L"Calibration", NULL, &valueType, NULL, &valueSize);
			if(valueType == REG_BINARY && valueSize == sizeof(calibration)) {
				if(0 == RegQueryValueExW(key, L"Calibration", NULL, &valueType, (LPBYTE)&calibration, &valueSize))
					calibrationMapping[i].isCalibrated = true;
			}

			RegCloseKey(key);
		}
	}
}

void RawJoystickDevice::HandleInput(RAWINPUT* input)
{
	HIDP_DATA data[64];
	ULONG dataCount = 64;
	if(HIDP_STATUS_SUCCESS != HidP_GetData(HidP_Input,
		data, &dataCount,
		m_InputReportProtocol,
		(PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid))
		throw core::Win32Exception(GetLastError());

	int axisCur = 0;
	int buttonCur = 0;

	core::Fill(m_NewButtonStates, false);
	for(ULONG i = 0; i < dataCount; ++i) {
		USHORT dataIndex = data[i].DataIndex;
		bool matched = false;
		while(!matched) {
			if(buttonCur < m_Buttons.Size() && m_Buttons[buttonCur].index == dataIndex) {
				m_NewButtonStates[buttonCur] = (data[i].On == TRUE);

				++buttonCur;
				matched = true;
			}

			if(axisCur < m_Axes.Size() && m_Axes[axisCur].index == dataIndex) {
				AxisEvent event;
				event.code = m_Axes[axisCur].code;

				int32_t min = m_Axes[axisCur].isCalibrated ? m_Axes[axisCur].logicalCalibratedMin : m_Axes[axisCur].logicalMin;
				int32_t max = m_Axes[axisCur].isCalibrated ? m_Axes[axisCur].logicalCalibratedMax : m_Axes[axisCur].logicalMax;

				if((int32_t)data[i].RawValue < min)
					data[i].RawValue = min;
				if((int32_t)data[i].RawValue > max)
					data[i].RawValue = max;

				if(m_Axes[axisCur].usagePage == 1 && m_Axes[axisCur].usage == 0x39) // Hat switch
					event.abs = float(data[i].RawValue - m_Axes[axisCur].logicalMin) / float(m_Axes[axisCur].logicalMax - m_Axes[axisCur].logicalMin + 1); // Map to [0, 1]
				else if(m_Axes[axisCur].usagePage == 1 && m_Axes[axisCur].usage == 0x36) // Slider
					event.abs = float(data[i].RawValue - min) / float(max - min); // Map to [0, 1]
				else // All the other things
					event.abs = (float(data[i].RawValue - min) * 2) / float(max - min) - 1.0f; // Map to [-1, 1]

				SendInputEvent(event);

				++axisCur;
				matched = true;
			}

			if(!matched) {
				int axisIndex = axisCur < m_Axes.Size() ? m_Axes[axisCur].index : 10000;
				int buttonIndex = buttonCur < m_Buttons.Size() ? m_Buttons[buttonCur].index : 10000;
				if(axisIndex < buttonIndex)
					axisCur++;
				else
					buttonCur++;
				if(axisIndex == 10000 && buttonIndex == 10000)
					break;
			}
		}
	}

	for(int i = 0; i < m_Buttons.Size(); ++i) {
		if(m_NewButtonStates[i] == m_ButtonStates[i])
			continue;
		m_ButtonStates[i] = m_NewButtonStates[i];

		ButtonEvent event;
		event.code = i;
		event.pressedDown = m_NewButtonStates[i];
		SendInputEvent(event);
	}
}

void RawJoystickDevice::InitDeviceDescription()
{
	// Only assign pointer at end of function.
	// So if a exception is thrown no partial desciption is created.
	auto desc = LUX_NEW(RawInputDeviceDescription);

	desc->name = GetDeviceName(m_NtHandle);
	desc->guid = GetDeviceGUID(m_RawInputHandle);
	desc->type = EDeviceType::Joystick;

	desc->buttonCount = m_Buttons.Size();
	for(int i = 0; i < desc->buttonCount; ++i) {
		const Button& button = m_Buttons[m_CodeHIDMapping[i]];
		EDeviceElementType elem_type = CombineFlags(
			EDeviceElementType::Input,
			EDeviceElementType::Button,
			button.isAbsolute ? EDeviceElementType::PushButton : EDeviceElementType::ToggleButton);
		desc->desc.EmplaceBack(button.name, elem_type);
	}

	desc->axesCount = m_Axes.Size();
	for(int i = 0; i < desc->axesCount; ++i) {
		const Axis& axis = m_Axes[m_CodeHIDMapping[m_Buttons.Size() + i]];
		EDeviceElementType elem_type = CombineFlags(
			EDeviceElementType::Input,
			EDeviceElementType::Axis,
			axis.isAbsolute ? EDeviceElementType::Abs : EDeviceElementType::Rel);
		if(axis.usagePage == 1 && axis.usage == 0x39)
			elem_type = CombineFlags(EDeviceElementType::Input, EDeviceElementType::Axis, EDeviceElementType::POV);
		desc->desc.EmplaceBack(axis.name, elem_type);
	}

	desc->areasCount = 0;

	m_Desc = desc;
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
