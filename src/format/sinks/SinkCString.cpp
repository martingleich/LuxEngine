#include "format/sinks/SinkCString.h"

namespace format
{
int cstring_sink::Write(Context&, const Context::SlicesT& slices, int flags)
{
	if(m_Str.maxSize == 0)
		return 0;

	int remaining = m_Str.maxSize - 1; // 1 for terminating zero.

	char* c = m_Str.string;
	for(auto& s : slices) {
		if(!remaining)
			break;
		int tocopy = s.size < remaining ? s.size : remaining;
		std::memcpy(c, s.data, tocopy);
		c += tocopy;
		remaining -= tocopy;
	}

	if(remaining && (flags & ESinkFlags::Newline) != 0)
		*c++ = '\n';

	*c++ = '\0';

	return int(c - m_Str.string) - 1;
}
}
