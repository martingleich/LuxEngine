#ifndef INCLUDED_FORMAT_SINK_OSTREAM_H
#define INCLUDED_FORMAT_SINK_OSTREAM_H
#include "../Sink.h"
#include <ostream>

namespace format
{

/** \addtogroup Sinks
@{
*/
//! Allows to write to std::ostream
class ostream_sink : public Sink
{
public:
	explicit ostream_sink(std::ostream& o) :
		m_Stream(o)
	{
	}

	FORMAT_API int Write(Context& ctx, const Context::SlicesT& slices, int flags) override;

private:
	std::ostream& m_Stream;
};

/** \cond UNDOCUMENTED */
template <>
struct sink_access<std::ostream>
{
	static ostream_sink Get(std::ostream& x) { return ostream_sink(x); }
};
/** \endcond */

/** @}*/
}
#endif // #ifndef INCLUDED_FORMAT_SINK_OSTREAM_H