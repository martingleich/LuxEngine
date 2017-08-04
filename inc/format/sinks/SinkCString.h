#ifndef INCLUDED_FORMAT_SINK_CSTRING_H
#define INCLUDED_FORMAT_SINK_CSTRING_H
#include "../Sink.h"
#include <limits.h>

namespace format
{

/** \addtogroup Sinks
@{
*/

//! Class to represent a safe c-string, which can't be accesed out of bounds
/**
Can and should be used as sink-type to access c-strings.
*/
struct SafeCString
{
	SafeCString(char* ptr, size_t _maxSize) :
		string(ptr),
		maxSize(_maxSize)
	{
	}

	char* string; // The address where the string data is written.
	size_t maxSize; // The maximal number of bytes available, including the NUL Character.
};

//! Allows to write to char*
class cstring_sink : public Sink
{
public:
	cstring_sink(SafeCString s, size_t collumn = 0) :
		Sink(collumn),
		m_Str(s)
	{
	}

	LUX_API size_t Write(Context& ctx, const Slice* firstSlice, int flags);

private:
	SafeCString m_Str;
};

/** \cond UNDOCUMENTED */
template <>
struct sink_access<SafeCString>
{
	static cstring_sink Get(const SafeCString& x) { return cstring_sink(x); }
};

template <>
struct sink_access<char*>
{
	static cstring_sink Get(char* x)
	{
		return cstring_sink(SafeCString(x, SIZE_MAX));
	}
};
/** \endcond */

/** @}*/
}
#endif // #ifndef INCLUDED_FORMAT_SINK_CSTRING_H