#include "SinkStdString.h"
#include <string>

namespace format
{
size_t stdstring_sink::Write(Context& ctx, const slice* firstSlice, int flags)
{
	if(ctx.stringType == StringType::CodePoint)
		throw not_implemeted_exception("Stringtype not supported.");

	m_Collumn = ctx.GetCollumn();

	(void)ctx;

	size_t size = 0;
	for(auto slice = firstSlice; slice; slice = slice->GetNext())
		size += slice->size;

	if((flags & ESinkFlags::Newline) != 0)
		++size;

	char* base = ctx.AllocByte(size);
	char* c = base;

	for(auto slice = firstSlice; slice; slice = slice->GetNext()) {
		memcpy(c, slice->data, slice->size);
		c += slice->size;
	}

	if((flags & ESinkFlags::Newline) != 0) {
		*c++ = '\n';
		m_Collumn = 0;
	}


	m_Str.assign(base, size);

	return size;
}

}
