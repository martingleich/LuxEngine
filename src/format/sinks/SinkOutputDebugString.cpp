#include "format/sinks/SinkOutputDebugString.h"
#ifdef FORMAT_WINDOWS
#include "format/UnicodeConversion.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace format
{

int OutputDebugString_sink::Write(Context& ctx, const Context::SlicesT& slices, int flags)
{
	int size = 1 + ctx.GetSize();
	if((flags & ESinkFlags::Newline) != 0)
		++size;

	std::vector<uint16_t> utf16Msg;
	utf16Msg.reserve(size);
	for(auto& s : slices)
		Utf8ToUtf16(s.data, s.size, utf16Msg);

	if(flags & ESinkFlags::Newline)
		utf16Msg.push_back('\n');

	utf16Msg.push_back(0);

	OutputDebugStringW((LPCWSTR)utf16Msg.data());

	return (size - 1);
}
}
#endif
