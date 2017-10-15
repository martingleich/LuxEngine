#ifndef INCLUDED_FORMAT_SINK_FILE_H
#define INCLUDED_FORMAT_SINK_FILE_H
#include "../Sink.h"
#include <stdio.h>

namespace format
{

/** \addtogroup Sinks
@{
*/

//! Allows to write to FILE*
class cfile_sink : public Sink
{
public:
	cfile_sink(FILE* f, size_t collumn = 0) :
		Sink(collumn),
		m_File(f)
	{
	}

	FORMAT_API virtual size_t Write(Context& ctx, const Slice* firstSlice, int flags);

private:
	FILE* m_File;
};

/** \cond UNDOCUMENTED */
template <>
struct sink_access<FILE*>
{
	static cfile_sink Get(FILE* x) { return cfile_sink(x); }
};
/** \endcond */

/** @}*/
}

#endif // #ifndef INCLUDED_FORMAT_SINK_FILE_H