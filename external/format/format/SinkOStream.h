#pragma once
#include "Sink.h"
#include <iostream>

namespace format
{

class ostream_sink : public sink
{
public:
	ostream_sink(std::ostream& o, size_t collumn=0) :
		sink(collumn),
		m_Stream(o)
	{
	}

	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags);

private:
	std::ostream& m_Stream;
};

template <>
struct sink_access<std::ostream>
{
	static ostream_sink Get(std::ostream& x) { return ostream_sink(x); }
};

}