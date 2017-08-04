#ifndef INCLUDED_FORMAT_SINK_STDSTRING_H
#define INCLUDED_FORMAT_SINK_STDSTRING_H
#include "../Sink.h"
#include <string>

namespace format
{

/** \addtogroup Sinks
@{
*/

//! Allows to write to std::string
class stdstring_sink : public Sink
{
public:
	stdstring_sink(std::string& s, size_t collumn = 0) :
		Sink(collumn),
		m_Str(s)
	{
	}

	LUX_API virtual size_t Write(Context& ctx, const Slice* firstSlice, int flags);

private:
	std::string& m_Str;
};

/** \cond UNDOCUMENTED */
template <>
struct sink_access<std::string>
{
	static stdstring_sink Get(std::string& x) { return stdstring_sink(x); }
};
/* \endcond */

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_SINK_STDSTRING_H