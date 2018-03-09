#include "format/sinks/SinkOutputDebugString.h"
#ifdef FORMAT_WINDOWS
#include "format/UnicodeConversion.h"
#include <Windows.h>

namespace format
{

size_t OutputDebugString_sink::Write(Context& ctx, const Slice* firstSlice, int flags)
{
	size_t size = 1 + ctx.GetSize();
	if((flags & ESinkFlags::Newline) != 0)
		++size;

	std::vector<uint16_t> utf16Msg;
	utf16Msg.reserve(size);
	for(auto slice = firstSlice; slice; slice = slice->GetNext())
		Utf8ToUtf16(slice->data, slice->size, utf16Msg);

	if(flags & ESinkFlags::Newline)
		utf16Msg.push_back('\n');

	utf16Msg.push_back(0);

	OutputDebugStringW((LPCWSTR)utf16Msg.data());

	return (size - 1);
}
}
#endif
