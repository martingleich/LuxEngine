#ifndef INCLUDED_LUX_FORMAT_H
#define INCLUDED_LUX_FORMAT_H
#include "format/Format.h"
#include "format/Sink.h"
#include "format/Converters.h"
#include "format/ConvertersHelper.h"
#include "core/lxString.h"

namespace lux
{
namespace core
{

class StringSink : public format::Sink
{
public:
	StringSink(core::String& s) :
		m_Str(s)
	{
	}

	int Write(format::Context& ctx, const format::Context::SlicesT& slices, int flags) override
	{
		(void)ctx;

		size_t size = ctx.GetSize();
		if((flags & format::ESinkFlags::Newline) != 0)
			++size;

		m_Str.Reserve(m_Str.Size() + (int)size);
		for(auto& s : slices)
			m_Str.Append(StringView(s.data, (int)s.size));

		if((flags & format::ESinkFlags::Newline) != 0)
			m_Str.Append(StringView("\n", 1));

		return size;
	}

private:
	core::String& m_Str;
};


inline void fmtPrint(format::Context& ctx, const core::String& s, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	ctx.AddSlice(s.Size(), s.Data());
}
inline void fmtPrint(format::Context& ctx, core::StringView s, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	ctx.AddSlice(s.Size(), s.Data());
}

} // namespace core
} // namespace lux

namespace format
{
template <>
struct sink_access<lux::core::String>
{
	static lux::core::StringSink Get(lux::core::String& x) { return lux::core::StringSink(x); }
};
}

#endif // #ifndef INCLUDED_LUX_FORMAT_H
