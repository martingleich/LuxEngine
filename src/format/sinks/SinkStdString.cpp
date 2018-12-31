#include "format/sinks/SinkStdString.h"
#include <string>

namespace format
{
int stdstring_sink::Write(Context& ctx, const Context::SlicesT& slices, int flags)
{
	int size = ctx.GetSize();
	if((flags & ESinkFlags::Newline) != 0)
		++size;

	m_Str.resize(size);
	char* c = &m_Str[0];

	for(auto& s : slices) {
		memcpy(c, s.data, s.size);
		c += s.size;
	}

	if((flags & ESinkFlags::Newline) != 0)
		*c++ = '\n';

	return size;
}

}
