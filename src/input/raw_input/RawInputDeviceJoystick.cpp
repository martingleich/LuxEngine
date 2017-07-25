#ifdef LUX_COMPILE_WITH_RAW_INPUT
#include "RawInputDeviceJoystick.h"

#include "core/lxAlgorithm.h"
#include "core/StringConverter.h"

#include "core/lxUnicodeConversion.h"
#include "Win32Exception.h"

#ifdef BREAK_ON_FAIL
#error BREAK_ON_FAIL makro already defined
#endif

#ifdef BREAK_ON_NULL
#error BREAK_ON_NULL makro already defined
#endif

#ifdef _DEBUG
#define BREAK_ON_FAIL(result, action) if(Failed(result = ((action)?EResult::Succeeded:EResult::Failed))) { printf("%s(%d) \"%s\" failed.\n", __FUNCTION__, __LINE__, #action); break;}
#define BREAK_ON_NULL(result, value, failError) if(!(value)) { printf("%s(%d) \"%s\" is invalid.\n", __FUNCTION__, __LINE__, #value); result = failError; break;}
#else
#define BREAK_ON_FAIL(result, action) if(Failed(result = ((action)?EResult::Succeeded:EResult::Failed))) { break;}
#define BREAK_ON_NULL(result, value, failError) if(!value) { result = failError; break;}
#endif

namespace lux
{
namespace input
{

RawJoystickDevice::RawJoystickDevice(InputSystem* system, HANDLE rawHandle) :
	RawInputDevice(system),
	m_RawInputHandle(NULL),
	m_NtHandle(NULL),
	m_ReportSize(0),
	m_InputReportProtocol(NULL)
{
	m_InputReportProtocol = NULL;
	m_NtHandle = NULL;
	m_RawInputHandle = rawHandle;

	// If the nt handle can't retrieved the device is likely not connected.
	// Has to be handled further up.
	m_NtHandle = GetDeviceHandle();

	m_Name = GetDeviceName();

	m_GUID = GetDeviceGUID(rawHandle);

	if(!HidD_GetPreparsedData(m_NtHandle, &m_InputReportProtocol))
		throw core::Win32Exception(GetLastError());

	HIDP_CAPS caps;
	if(HIDP_STATUS_SUCCESS != HidP_GetCaps(m_InputReportProtocol, &caps))
		throw core::Win32Exception(GetLastError());

	m_ReportSize = caps.InputReportByteLength;

	core::Array<HIDP_BUTTON_CAPS> buttonCaps;
	size_t buttonCount = 0;
	GetButtonCaps(caps, buttonCaps, buttonCount);

	core::Array<HIDP_VALUE_CAPS> axesCaps;
	size_t axesCount = 0;
	GetAxesCaps(caps, axesCaps, axesCount);

	MappingAndCalibration directInputAxisMapping[7] = {};
	Mapping directInputButtonMapping[128] = {};

	for(auto it = axesCaps.First(); it != axesCaps.End(); ++it) {
		if(it->UsagePage == HID_USAGE_PAGE_GENERIC) {
			USAGE firstUsage = it->Range.UsageMin;
			USAGE lastUsage = it->Range.UsageMax;
			for(USAGE currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage) {
				int index = currentUsage - HID_USAGE_GENERIC_X;
				if(index >= 0 && index < 7) {
					directInputAxisMapping[index].usagePage = HID_USAGE_PAGE_GENERIC;
					directInputAxisMapping[index].usage = currentUsage;
				}
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
	}

	for(auto it = buttonCaps.First(); it != buttonCaps.End(); ++it) {
		if(it->UsagePage == HID_USAGE_PAGE_BUTTON) {
			USAGE firstUsage = it->Range.UsageMin;
			USAGE lastUsage = it->Range.UsageMax;
			for(USAGE currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage) {
				directInputButtonMapping[currentUsage - 1].usagePage = HID_USAGE_PAGE_BUTTON;
				directInputButtonMapping[currentUsage - 1].usage = currentUsage;
			}
		}
	}

	HIDD_ATTRIBUTES deviceAttributes;
	if(FALSE != HidD_GetAttributes(m_NtHandle, &deviceAttributes)) {
		LoadDirectInputMapping(true, (Mapping*)directInputAxisMapping, 7, sizeof(directInputAxisMapping[0]), deviceAttributes);
		LoadDirectInputMapping(false, directInputButtonMapping, 128, sizeof(directInputButtonMapping[0]), deviceAttributes);

		LoadDirectInputAxisCalibration(directInputAxisMapping, 7, deviceAttributes);
	}

	for(auto it = axesCaps.First(); it != axesCaps.End(); ++it) {
		USAGE firstUsage = it->Range.UsageMin;
		USAGE lastUsage = it->Range.UsageMax;
		for(USAGE currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage) {
			WORD currentIndex = it->Range.DataIndexMin + (currentUsage - firstUsage);

			bool isCalibrated = false;
			int32_t calibratedMin = 0;
			int32_t calibratedMax = 0;
			int32_t calibratedCenter = 0;
			wchar_t const *toName = L"";

			for(size_t i = 0; i < ARRAYSIZE(directInputAxisMapping); ++i) {
				auto& mapping = directInputAxisMapping[i];

				if(it->UsagePage == mapping.usagePage && currentUsage == mapping.usage) {
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
			axis.usagePage = it->UsagePage;
			axis.usage = currentUsage;
			axis.index = currentIndex;
			axis.reportID = it->ReportID;
			axis.logicalMin = it->LogicalMin;
			axis.logicalMax = it->LogicalMax;
			axis.isCalibrated = isCalibrated;
			axis.isAbsolute = (it->IsAbsolute == TRUE);
			if(axis.isCalibrated) {
				axis.logicalCalibratedMin = calibratedMin;
				axis.logicalCalibratedMax = calibratedMax;
				axis.logicalCalibratedCenter = calibratedCenter;
			}

			axis.name = core::UTF16ToString(toName);

			m_Axes.PushBack(axis);
		}
	}

	for(auto it = buttonCaps.First(); it != buttonCaps.End(); ++it) {
		USAGE firstUsage = it->Range.UsageMin;
		USAGE lastUsage = it->Range.UsageMax;
		for(USAGE currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage) {
			WORD currentIndex = it->Range.DataIndexMin + (currentUsage - firstUsage);
			wchar_t const* toName = L"";
			for(size_t i = 0; i < ARRAYSIZE(directInputButtonMapping); ++i) {
				auto& mapping = directInputButtonMapping[i];

				if(it->UsagePage == mapping.usagePage && currentUsage == mapping.usage) {
					toName = mapping.name;

					mapping.usage = 0;
					break;
				}
			}

			Button button;
			button.usagePage = it->UsagePage;
			button.usage = currentUsage;
			button.reportID = it->ReportID;
			button.index = currentIndex;
			button.name = core::UTF16ToString(toName);
			button.isAbsolute = (it->IsAbsolute == TRUE);

			m_Buttons.PushBack(button);
		}
	}

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

	m_ButtonStates.Resize(m_Buttons.Size(), false);
	m_NewButtonStates.Resize(m_Buttons.Size(), false);
}

RawJoystickDevice::~RawJoystickDevice()
{
	if(m_InputReportProtocol)
		HidD_FreePreparsedData(m_InputReportProtocol);

	if(m_NtHandle)
		CloseHandle(m_NtHandle);
}

HANDLE RawJoystickDevice::GetDeviceHandle()
{
	String path = GetDevicePath(m_RawInputHandle);

	HANDLE ntHandle = CreateFileW(core::StringToUTF16W(path), 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0, NULL);

	if(ntHandle == INVALID_HANDLE_VALUE)
		throw core::Win32Exception(GetLastError());

	return ntHandle;
}

String RawJoystickDevice::GetDeviceName()
{
	const size_t max_size = 127;

	wchar_t nameBuffer[max_size];

	size_t len = 0;
	if(HidD_GetManufacturerString(m_NtHandle, nameBuffer, max_size * 2)) {
		len = wcslen(nameBuffer);
		nameBuffer[len++] = ' ';
	}

	if(HidD_GetProductString(m_NtHandle, nameBuffer + len, ULONG(max_size - len) * 2)) {
		len = wcslen(nameBuffer);
	}

	if(len == 0)
		return "(unknown)";
	else
		return core::UTF16ToString(nameBuffer);
}

void RawJoystickDevice::GetButtonCaps(const HIDP_CAPS& deviceCaps, core::Array<HIDP_BUTTON_CAPS>& buttonCaps, size_t& buttonCount)
{
	buttonCaps.Clear();
	buttonCaps.Resize(deviceCaps.NumberInputButtonCaps);
	buttonCount = 0;

	USHORT numberInputButtonCaps = deviceCaps.NumberInputButtonCaps;
	if(HIDP_STATUS_SUCCESS != HidP_GetButtonCaps(HidP_Input,
		&*buttonCaps.First(),
		&numberInputButtonCaps,
		m_InputReportProtocol))
		throw core::Win32Exception(GetLastError());

	for(auto it = buttonCaps.First(); it != buttonCaps.End(); ++it) {
		if(it->IsRange) {
			buttonCount += it->Range.UsageMax - it->Range.UsageMin + 1;
		} else {
			it->Range.UsageMin = it->NotRange.Usage;
			it->Range.UsageMax = it->NotRange.Usage;
			it->Range.DataIndexMin = it->NotRange.DataIndex;
			it->Range.DataIndexMax = it->NotRange.DataIndex;
			it->IsRange = 1;
			++buttonCount;
		}
	}
}

void RawJoystickDevice::GetAxesCaps(const HIDP_CAPS& deviceCaps, core::Array<HIDP_VALUE_CAPS>& valueCaps, size_t& valueCount)
{
	valueCaps.Clear();
	valueCaps.Resize(deviceCaps.NumberInputValueCaps);
	valueCount = 0;

	USHORT numberInputValueCaps = deviceCaps.NumberInputValueCaps;
	if(HIDP_STATUS_SUCCESS != HidP_GetValueCaps(HidP_Input,
		&*valueCaps.First(),
		&numberInputValueCaps,
		m_InputReportProtocol))
		throw core::Win32Exception(GetLastError());

	for(auto it = valueCaps.First(); it != valueCaps.End(); ++it) {
		if(it->IsRange) {
			valueCount += it->Range.UsageMax - it->Range.UsageMin + 1;
		} else {
			it->Range.UsageMin = it->NotRange.Usage;
			it->Range.UsageMax = it->NotRange.Usage;
			it->Range.DataIndexMin = it->NotRange.DataIndex;
			it->Range.DataIndexMax = it->NotRange.DataIndex;
			if(it->LogicalMax < it->LogicalMin) {
				it->LogicalMin = it->LogicalMin > 0 ? it->LogicalMin : 0;
				it->LogicalMax = (1 << it->BitSize);
			}
			if((1 << it->BitSize) < it->LogicalMax)
				it->LogicalMax = (1 << it->BitSize);

			it->IsRange = 1;
			++valueCount;
		}
	}
}

void RawJoystickDevice::LoadDirectInputMapping(bool isAxis, Mapping* mappings, size_t mappingCount, size_t offset, const HIDD_ATTRIBUTES& attribs)
{
	wchar_t* subType = isAxis ? L"Axes" : L"Buttons";

	wchar_t path[128];
	wsprintfW(path, L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_%04X&PID_%04X\\%s\\", attribs.VendorID, attribs.ProductID, subType);

	size_t numberBegin = wcslen(path);
	for(size_t i = 0; i < mappingCount; ++i) {
		wsprintfW(path + numberBegin, L"%d", i);

		HKEY key = NULL;
		if(0 != RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_READ, &key))
			continue;

		DWORD valueType;
		DWORD valueSize;

		Mapping& curMap = *(Mapping*)((char*)mappings + i*offset);

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

void RawJoystickDevice::LoadDirectInputAxisCalibration(MappingAndCalibration* calibrationMapping, size_t mappingCount, const HIDD_ATTRIBUTES& attribs)
{
	wchar_t path[128];
	wsprintfW(path, L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_%04X&PID_%04X\\Calibration\\0\\Type\\Axes\\", attribs.VendorID, attribs.ProductID);

	size_t numberBegin = wcslen(path);
	for(size_t i = 0; i < mappingCount; ++i) {
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

	size_t axisCur = 0;
	size_t buttonCur = 0;

	core::Fill(m_NewButtonStates.First(), m_NewButtonStates.End(), false);
	for(size_t i = 0; i < dataCount; ++i) {
		USHORT dataIndex = data[i].DataIndex;
		bool matched = false;
		while(!matched) {
			if(buttonCur < m_Buttons.Size() && m_Buttons[buttonCur].index == dataIndex) {
				m_NewButtonStates[buttonCur] = (data[i].On == TRUE);

				++buttonCur;
				matched = true;
			}

			if(axisCur < m_Axes.Size() && m_Axes[axisCur].index == dataIndex) {
				Event event;
				event.source = EEventSource::Joystick;
				event.type = EEventType::Axis;
				event.internal_abs_only = true;
				event.internal_rel_only = false;
				event.axis.code = (EAxisCode)m_Axes[axisCur].code;

				int32_t min = m_Axes[axisCur].isCalibrated ? m_Axes[axisCur].logicalCalibratedMin : m_Axes[axisCur].logicalMin;
				int32_t max = m_Axes[axisCur].isCalibrated ? m_Axes[axisCur].logicalCalibratedMax : m_Axes[axisCur].logicalMax;

				if((int32_t)data[i].RawValue < min)
					data[i].RawValue = min;
				if((int32_t)data[i].RawValue > max)
					data[i].RawValue = max;

				if(m_Axes[axisCur].usagePage == 1 && m_Axes[axisCur].usage == 0x39) // Hat switch
					event.axis.abs = float(data[i].RawValue - m_Axes[axisCur].logicalMin) / float(m_Axes[axisCur].logicalMax - m_Axes[axisCur].logicalMin + 1); // Map to [0, 1]
				else if(m_Axes[axisCur].usagePage == 1 && m_Axes[axisCur].usage == 0x36) // Slider
					event.axis.abs = float(data[i].RawValue - min) / float(max - min); // Map to [0, 1]
				else // All the other things
					event.axis.abs = (float(data[i].RawValue - min) * 2) / float(max - min) - 1.0f; // Map to [-1, 1]

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

	for(size_t i = 0; i < m_Buttons.Size(); ++i) {
		if(m_NewButtonStates[i] != m_ButtonStates[i]) {
			Event event;
			event.source = EEventSource::Joystick;
			event.type = EEventType::Button;
			event.internal_abs_only = false;
			event.internal_rel_only = false;
			event.button.code = (EKeyCode)(u32)i;
			event.button.pressedDown = m_NewButtonStates[i];
			event.button.state = m_NewButtonStates[i];
			SendInputEvent(event);
			m_ButtonStates[i] = m_NewButtonStates[i];
		}
	}
}

EEventSource RawJoystickDevice::GetType() const
{
	return EEventSource::Joystick;
}

size_t RawJoystickDevice::GetElementCount(EEventType type) const
{
	if(type == EEventType::Button)
		return m_Buttons.Size();
	else if(type == EEventType::Axis)
		return m_Axes.Size();
	else
		return 0;
}

RawJoystickDevice::ElemDesc RawJoystickDevice::GetElementDesc(EEventType type, u32 code) const
{
	if(type == EEventType::Button) {
		const Button& button = m_Buttons[m_CodeHIDMapping[code]];
		EElementType elem_type = EElementType::Input | EElementType::Button | (button.isAbsolute ? EElementType::PushButton : EElementType::ToggleButton);
		return ElemDesc(button.name, button.usagePage, button.usage, elem_type);
	} else if(type == EEventType::Axis) {
		const Axis& axis = m_Axes[m_CodeHIDMapping[m_Buttons.Size() + code]];
		EElementType elem_type = EElementType::Input | EElementType::Axis | (axis.isAbsolute ? EElementType::Abs : EElementType::Rel);
		if(axis.usagePage == 1 && axis.usage == 0x39)
			elem_type = EElementType::Input | EElementType::Axis | EElementType::POV;
		return ElemDesc(axis.name, axis.usagePage, axis.usage, elem_type);
	} else {
		static const String name = "(unknown)";
		return ElemDesc(name, 0, 0, EElementType::Other);
	}
}

}
}
#endif // LUX_COMPILE_WITH_RAW_INPUT
