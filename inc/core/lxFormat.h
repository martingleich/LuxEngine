#ifndef INCLUDED_LX_FORMAT_H
#define INCLUDED_LX_FORMAT_H
#include "format/ConvInternal.h"
#include "format/Format.h"
#include "format/Sink.h"
#include "format/Converters.h"
#include "core/lxString.h"

namespace lux
{
namespace core
{

class StringSink : public format::Sink
{
public:
	StringSink(core::String& s, size_t collumn=0) :
		Sink(collumn),
		m_Str(s)
	{
	}

	virtual size_t Write(format::Context& ctx, const format::Slice* firstSlice, int flags)
	{
		if(ctx.stringType == format::StringType::CodePoint)
			return 0;

		m_Collumn = ctx.GetCollumn();

		(void)ctx;

		size_t size = 0;
		for(auto slice = firstSlice; slice; slice = slice->GetNext())
			size += slice->size;

		if((flags & format::ESinkFlags::Newline) != 0)
			++size;

		m_Str.Reserve(m_Str.Size() + size);
		for(auto slice = firstSlice; slice; slice = slice->GetNext())
			m_Str.AppendRaw(slice->data, slice->size);

		if((flags & format::ESinkFlags::Newline) != 0) {
			m_Str.Append("\n");
			m_Collumn = 0;
		}

		return size;
	}

private:
	core::String& m_Str;
};

inline void conv_data(format::Context& ctx, const core::String& s, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	format::ConvertAddString(ctx, format::StringType::Unicode, s.Data_c(), s.Size());
}

}
}

#endif // #ifndef INCLUDED_LX_FORMAT_H