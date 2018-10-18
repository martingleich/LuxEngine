#ifndef INCLUDED_LUX_WIN32_HELPER
#define INCLUDED_LUX_WIN32_HELPER

#ifdef LUX_WINDOWS

#include "platform/StrippedWindows.h"
#include "core/lxFormat.h"
#include "core/lxString.h"
#include "core/lxUnicodeConversion.h"
#include "core/lxException.h"

namespace lux
{
inline core::String GetWin32ErrorString(DWORD error)
{
	lux::core::String out;
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
			out = lux::core::UTF16ToString(textBytes, -1);
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
		error(_error)
	{
	}
	ExceptionSafeString What() const { return (StringView)GetWin32ErrorString(error); }

	DWORD error;
};

struct LogWin32Error
{
	DWORD error;
	LogWin32Error(DWORD e) : error(e)
	{
	}
};

inline void fmtPrint(format::Context& ctx, const LogWin32Error& v, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);

	using namespace format;
	lux::core::String str = GetWin32ErrorString(v.error);
	ctx.AddSlice(str.Size(), str.Data());
}

}
}

#endif

#endif // #ifndef INCLUDED_LUX_WIN32_HELPER
