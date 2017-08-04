#ifndef INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H
#define INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H
#include "../Sink.h"
#ifdef LUX_WINDOWS

namespace format
{

/** \addtogroup Sinks
@{
*/

//! Allows to write to the debugger
/**
Currently only available for Windows
*/
class OutputDebugString_sink : public Sink
{
public:
	OutputDebugString_sink(size_t collumn = 0) :
		Sink(collumn)
	{
	}
	LUX_API virtual size_t Write(Context& ctx, const Slice* firstSlice, int flags);
};

//! Helper function to create a debug writer
inline OutputDebugString_sink OutputDebugString(size_t collumn = 0)
{
	return OutputDebugString_sink(collumn);
}

/** @}*/
}

#endif // #ifndef LUX_WINDOWS
#endif // #ifndef INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H