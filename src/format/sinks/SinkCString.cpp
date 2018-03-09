#include "format/sinks/SinkCString.h"

namespace format
{
size_t cstring_sink::Write(Context&, const Slice* firstSlice, int flags)
{
	if(m_Str.maxSize == 0)
		return 0;

	size_t remaining = m_Str.maxSize - 1; // 1 for terminating zero.

	char* c = m_Str.string;
	for(auto slice = firstSlice; remaining && slice; slice = slice->GetNext()) {
		size_t tocopy = slice->size < remaining ? slice->size : remaining;
		memcpy(c, slice->data, tocopy);
		c += tocopy;
		remaining -= tocopy;
	}

	if(remaining  && (flags & ESinkFlags::Newline) != 0)
		*c++ = '\n';

	*c++ = '\0';

	return (c - m_Str.string) - 1;
}
}
