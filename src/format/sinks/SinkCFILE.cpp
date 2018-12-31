#include "format/sinks/SinkCFILE.h"

namespace format
{
int cfile_sink::Write(Context&, const Context::SlicesT& slices, int flags)
{
	int size = 0;
	for(auto& s : slices) {
		if(fwrite(s.data, s.size, 1, m_File) != 1)
			return size;
		size += s.size;
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
