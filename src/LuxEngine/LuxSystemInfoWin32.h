#ifndef INCLUDED_LUX_SYSTEM_INFO_WIN32_H
#define INCLUDED_LUX_SYSTEM_INFO_WIN32_H
#include "LuxConfig.h"
#ifdef LUX_WINDOWS
#include "LuxEngine/LuxSystemInfo.h"
#include "platform/StrippedWindows.h"
#include "core/lxUnicodeConversion.h"

namespace lux
{

class LuxSystemInfoWin32 : public LuxSystemInfo
{
public:
	LuxSystemInfoWin32()
	{
		QueryProcInfo();
	}

	bool GetProcessorName(core::String& displayName)
	{
		if(m_ProcessorName.IsEmpty())
			return false;
		displayName = m_ProcessorName;
		return true;
	}

	bool GetProcessorSpeed(int& speedInMhz)
	{
		speedInMhz = m_ProcessorSpeed;
		return m_ProcessorSpeed != 0;
	}

	bool GetProcessorCount(int& count)
	{
		count = m_ProcessorCount;
		return count != 0;
	}

	bool GetTotalRAM(int& ramInMB)
	{
		MEMORYSTATUS memStat;
		GlobalMemoryStatus(&memStat);

		ramInMB = (int)(memStat.dwTotalPhys >> 10); // Is given in kb
		return true;
	}

	bool GetAvailableRAM(int& ramInMB)
	{
		MEMORYSTATUS memStat;
		GlobalMemoryStatus(&memStat);

		ramInMB = (int)(memStat.dwAvailPhys >> 10); // Is given in kb
		return true;
	}

	bool GetPrimaryScreenResolution(math::Dimension2I& dimension)
	{
		dimension.width = GetSystemMetrics(SM_CXSCREEN);
		dimension.height = GetSystemMetrics(SM_CYSCREEN);
		return  dimension.width != 0 && dimension.height != 0;
	}

private:
	void QueryProcInfo()
	{
		m_ProcessorCount = 0;
		m_ProcessorName = "";
		m_ProcessorSpeed = 0;

		// Returns 0 on failure
		m_ProcessorCount = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);

		LSTATUS status;

		HKEY key;
		status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
			L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
			0, KEY_READ, &key);

		if(status != ERROR_SUCCESS)
			return;

		DWORD size;
		DWORD type;
		{
			DWORD speed = 0;
			size = sizeof(speed);
			status = RegQueryValueExW(key, L"~MHz", NULL, &type, (LPBYTE)&speed, &size);
			if(status == ERROR_SUCCESS && type == REG_DWORD)
				m_ProcessorSpeed = speed;

			core::Array<u16> nameBuffer;

			nameBuffer.Resize(64); // Just some number
			do {
				size = (DWORD)nameBuffer.Size();
				status = RegQueryValueExW(key, L"ProcessorNameString", NULL, &type, (LPBYTE)nameBuffer.Data(), &size);
				if(status == ERROR_SUCCESS && type == REG_SZ) {
					m_ProcessorName = core::UTF16ToString(nameBuffer.Data());
					break;
				}
				if(status == ERROR_MORE_DATA)
					nameBuffer.Resize(nameBuffer.Size() * 2);
			} while(status == ERROR_MORE_DATA);
		}

		RegCloseKey(key);
	}

private:
	core::String m_ProcessorName;
	int m_ProcessorSpeed;
	int m_ProcessorCount;
};

} // namespace lux

#endif // #ifdef LUX_WINDOWS
#endif // #ifndef INCLUDED_LUX_SYSTEM_INFO_WIN32_H