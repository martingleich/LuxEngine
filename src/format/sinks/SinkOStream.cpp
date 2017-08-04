#include "format/sinks/SinkOStream.h"

namespace format
{
size_t ostream_sink::Write(Context& ctx, const Slice* firstSlice, int flags)
{
	if(ctx.stringType == StringType::CodePoint)
		throw not_implemeted_exception("Stringtype is not supported.");

	m_Collumn = ctx.GetCollumn();

	size_t size = 0;
	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		m_Stream.write(slice->data, slice->size);
		size += slice->size;
	}

	if(flags & ESinkFlags::Newline) {
		m_Stream.write("\n", 1);
		m_Collumn = 0;
		size += 1;
	}

	if(flags & ESinkFlags::Flush)
		m_Stream.flush();

	return size;
}
}