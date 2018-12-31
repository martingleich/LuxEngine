#ifndef INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H
#define INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H
#include "../Sink.h"
#ifdef FORMAT_WINDOWS

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
	FORMAT_API int Write(Context& ctx, const Context::SlicesT& slices, int flags) override;
};

//! Helper function to create a debug writer
inline OutputDebugString_sink OutputDebugString()
{
	return OutputDebugString_sink();
}

/** @}*/
}

#endif // #ifndef FORMAT_WINDOWS
#endif // #ifndef INCLUDED_FORMAT_SINK_OUTPUTDEBUGSTRING_H