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
	StringSink(core::String& s) :
		m_Str(s)
	{
	}

	virtual size_t Write(format::Context& ctx, const format::Context::SlicesT& slices, int flags)
	{
		(void)ctx;

		size_t size = ctx.GetSize();
		if((flags & format::ESinkFlags::Newline) != 0)
			++size;

		m_Str.Reserve(m_Str.Size() + size);
		for(auto& s : slices)
			m_Str.AppendRaw(s.data, s.size);

		if((flags & format::ESinkFlags::Newline) != 0)
			m_Str.Append("\n");

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

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_FORMAT_H