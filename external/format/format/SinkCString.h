#pragma once
#include "Sink.h"

namespace format
{

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

class cstring_sink : public sink
{
public:
	cstring_sink(SafeCString s, size_t collumn=0) :
		sink(collumn),
		m_Str(s)
	{
	}

	size_t Write(Context& ctx, const slice* firstSlice, int flags);

private:
	SafeCString m_Str;
};

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
		return cstring_sink(SafeCString(x, std::numeric_limits<decltype(SafeCString::maxSize)>::max()));
	}
};

}