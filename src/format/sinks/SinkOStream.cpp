#include "format/sinks/SinkOStream.h"

namespace format
{
size_t ostream_sink::Write(Context&, const Context::SlicesT& slices, int flags)
{
	size_t size = 0;
	for(auto& s : slices) {
		m_Stream.write(s.data, s.size);
		size += s.size;
	}

	if(flags & ESinkFlags::Newline) {
		m_Stream.write("\n", 1);
		size += 1;
	}

	if(flags & ESinkFlags::Flush)
		m_Stream.flush();

	return size;
}
}