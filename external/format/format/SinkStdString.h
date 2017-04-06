#pragma once
#include "Sink.h"
#include <string>

namespace format
{

class stdstring_sink : public sink
{
public:
	stdstring_sink(std::string& s, size_t collumn=0) :
		sink(collumn),
		m_Str(s)
	{
	}

	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags);

private:
	std::string& m_Str;
};

template <>
struct sink_access<std::string>
{
	static stdstring_sink Get(std::string& x) { return stdstring_sink(x); }
};

}
