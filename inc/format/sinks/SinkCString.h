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
	SafeCString(char* ptr, int _maxSize) :
		string(ptr),
		maxSize(_maxSize)
	{
	}

	char* string; // The address where the string data is written.
	int maxSize; // The maximal number of bytes available, including the NUL Character.
};

//! Allows to write to char*
class cstring_sink : public Sink
{
public:
	explicit cstring_sink(SafeCString s) :
		m_Str(s)
	{
	}

	FORMAT_API int Write(Context& ctx, const Context::SlicesT& slices, int flags) override;

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
	static cstring_sink Get(char* x) { return cstring_sink(SafeCString(x, INT_MAX)); }
};
template <int SIZE>
struct sink_access<char[SIZE]>
{
	static cstring_sink Get(char* x) { return cstring_sink(SafeCString(x, SIZE)); }
};
/** \endcond */

/** @}*/
}
#endif // #ifndef INCLUDED_FORMAT_SINK_CSTRING_H