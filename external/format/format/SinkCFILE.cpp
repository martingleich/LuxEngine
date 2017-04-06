#include "SinkCFILE.h"

namespace format
{
size_t cfile_sink::Write(Context& ctx, const slice* firstSlice, int flags)
{
	if(ctx.stringType == StringType::CodePoint)
		throw not_implemeted_exception("Stringtype is not supported.");

	m_Collumn = ctx.GetCollumn();

	size_t size = 0;
	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		fwrite(slice->data, slice->size, 1, m_File);
		size += slice->size;
	}

	if(flags & ESinkFlags::Newline) {
		fwrite("\n", 1, 1, m_File);
		m_Collumn = 0;
		size += 1;
	}

	if(flags & ESinkFlags::Flush)
		fflush(m_File);

	return size;
}
}
