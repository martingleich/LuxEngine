#include "format/sinks/SinkOStream.h"

namespace format
{
size_t ostream_sink::Write(Context&, const Slice* firstSlice, int flags)
{
	size_t size = 0;
	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		m_Stream.write(slice->data, slice->size);
		size += slice->size;
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