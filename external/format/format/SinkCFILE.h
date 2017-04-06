#pragma once
#include "Sink.h"
#include <stdio.h>

namespace format
{

class cfile_sink : public sink
{
public:
	cfile_sink(FILE* f, size_t collumn=0) :
		sink(collumn),
		m_File(f)
	{
	}

	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags);

private:
	FILE* m_File;
};

template <>
struct sink_access<FILE*>
{
	static cfile_sink Get(FILE* x) { return cfile_sink(x); }
};

}
