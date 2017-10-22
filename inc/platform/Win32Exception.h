#ifndef INCLUDED_LX_WIN32_HELPER
#define INCLUDED_LX_WIN32_HELPER

#ifdef LUX_WINDOWS

#include "StrippedWindows.h"
#include "core/lxFormat.h"
#include "core/lxString.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxException.h"

namespace lux
{
inline lux::String GetWin32ErrorString(DWORD error)
{
	lux::String out;
	if(NOERROR != error) {
		const DWORD formatControl =
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM;

		LPWSTR textBuffer = NULL;
		DWORD count = FormatMessageW(formatControl,
			NULL,
			error,
			0,
			textBuffer,
			0,
			NULL);
		const char* textBytes = (const char*)textBuffer;
		if(count != 0)
			out = lux::core::UTF16ToString(textBytes);
		else
			out = "Unknown error";
		HeapFree(GetProcessHeap(), 0, textBuffer);
	}

	return out;
}

namespace core
{
struct Win32Exception : RuntimeException
{
	explicit Win32Exception(DWORD _error) :
		RuntimeException(GetWin32ErrorString(_error).Data()),
		error(_error)
	{
	}

	DWORD error;
};

struct LogWin32Error
{
	DWORD error;
	LogWin32Error(DWORD e) : error(e)
	{
	}
};

inline String GetWin32ErrorString(DWORD error)
{
	String out;
	if(NOERROR != error) {
		const DWORD formatControl =
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM;

		LPWSTR textBuffer = NULL;
		DWORD count = FormatMessageW(formatControl,
			NULL,
			error,
			0,
			textBuffer,
			0,
			NULL);
		const char* textBytes = (const char*)textBuffer;
		if(count != 0)
			out = lux::core::UTF16ToString(textBytes);
		else
			out = "Unknown error";
		HeapFree(GetProcessHeap(), 0, textBuffer);
	}

	return out;
}

inline void conv_data(format::Context& ctx, const LogWin32Error& v, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);

	using namespace format;
	lux::String str = GetWin32ErrorString(v.error);
	format::CopyConvertAddString(ctx, format::StringType::Unicode, str.Data_c(), str.Size());
}

}
}

#endif

#endif // #ifndef INCLUDED_LX_WIN32_HELPER