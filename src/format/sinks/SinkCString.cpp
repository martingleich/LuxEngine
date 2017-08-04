#include "format/sinks/SinkCString.h"

namespace format
{
size_t cstring_sink::Write(Context& ctx, const Slice* firstSlice, int flags)
{
	if(ctx.stringType == StringType::CodePoint)
		throw not_implemeted_exception("Stringtype is not supported.");

	if(m_Str.maxSize == 0)
		return 0;

	m_Collumn = ctx.GetCollumn();

	size_t remaining = m_Str.maxSize - 1; // 1 for terminating zero.
	size_t size = 1;
	for(auto slice = firstSlice; slice; slice = slice->GetNext())
		size += slice->size;

	if((flags & ESinkFlags::Newline) != 0)
		++size;

	char* c = m_Str.string;

	for(auto slice = firstSlice; remaining && slice; slice = slice->GetNext()) {
		size_t tocopy = slice->size < remaining ? slice->size : remaining;
		memcpy(c, slice->data, tocopy);
		c += tocopy;
		remaining -= tocopy;
	}

	if(remaining  && (flags & ESinkFlags::Newline) != 0) {
		*c++ = '\n';
		m_Collumn = 0;
	}

	*c++ = '\0';

	return (size - 1);
}
}
