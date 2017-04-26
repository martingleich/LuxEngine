#ifndef INCLUDED_LX_FORMAT_H
#define INCLUDED_LX_FORMAT_H
#include "../external/format/format/Format.h"
#include "../external/format/format/Sink.h"
#include "../external/format/format/Converters.h"
#include "core/lxString.h"

namespace lux
{
namespace core
{

class string_sink : public format::sink
{
public:
	string_sink(string& s, size_t collumn=0) :
		sink(collumn),
		m_Str(s)
	{
	}

	virtual size_t Write(format::Context& ctx, const format::slice* firstSlice, int flags)
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
	string& m_Str;
};

}

inline void conv_data(format::Context& ctx, const string& s, format::Placeholder& placeholder)
{
	using namespace format;
	LUX_UNUSED(placeholder);
	ConvertAddString(ctx, StringType::Unicode, s.Data_c(), s.Size());
}

}

#endif // #ifndef INCLUDED_LX_FORMAT_H