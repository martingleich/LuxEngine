#pragma once
#include "Sink.h"

#ifdef FORMAT_WINDOWS
namespace format
{

class OutputDebugString_sink : public sink
{
public:
	OutputDebugString_sink(size_t collumn=0) :
		sink(collumn)
	{}
	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags);
};

inline OutputDebugString_sink OutputDebugString(size_t collumn=0)
{
	return OutputDebugString_sink(collumn);
}

}

#endif // FORMAT_WINDOWS
