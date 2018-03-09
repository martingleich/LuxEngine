#include "format/sinks/SinkCFILE.h"

namespace format
{
size_t cfile_sink::Write(Context&, const Slice* firstSlice, int flags)
{
	size_t size = 0;
	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		if(fwrite(slice->data, slice->size, 1, m_File) != 1)
			return size;
		size += slice->size;
	}

	if(flags & ESinkFlags::Newline) {
		if(fwrite("\n", 1, 1, m_File) != 1)
			return size;
		size += 1;
	}

	if(flags & ESinkFlags::Flush)
		fflush(m_File);

	return size;
}
}
