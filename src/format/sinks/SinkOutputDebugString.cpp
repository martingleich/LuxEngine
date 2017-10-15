#ifdef FORMAT_WINDOWS
#include "format/sinks/SinkOutputDebugString.h"
#include "format/UnicodeConversion.h"
#include <Windows.h>

namespace format
{

size_t OutputDebugString_sink::Write(Context& ctx, const Slice* firstSlice, int flags)
{
	m_Collumn = ctx.GetCollumn();

	size_t size = 1;
	for(auto slice = firstSlice; slice; slice = slice->GetNext())
		size += slice->size;

	if((flags & ESinkFlags::Newline) != 0)
		++size;

	char* str = ctx.AllocByte(size);
	char* c = str;

	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		memcpy(c, slice->data, slice->size);
		c += slice->size;
	}

	if(flags & ESinkFlags::Newline) {
		*c++ = '\n';
		m_Collumn = 0;
	}

	*c++ = 0;

	std::vector<uint16_t> utf16Msg;
	if(ctx.stringType == StringType::Ascii || ctx.stringType == StringType::Unicode)
		utf16Msg = Utf8ToUtf16(str, size);
	else if(ctx.stringType == StringType::CodePoint)
		utf16Msg = CodePointsToUtf16((const uint32_t*)str, size / 4);
	else
		throw not_implemeted_exception("Stringtype not supported.");

	OutputDebugStringW((LPCWSTR)utf16Msg.data());

	return (size-1);
}
}
#endif
