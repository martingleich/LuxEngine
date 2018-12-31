#ifndef INCLUDED_FORMAT_CONVERTERS_H
#define INCLUDED_FORMAT_CONVERTERS_H
#include "format/Context.h"

namespace format
{

/*
The fmtPrint function is called to convert a placeholder-argument to a string.
Each fmtPrint function, must convert it's argument to a string and
add it to the Context.
There can be additional memory allocation.
If a conversion is not possible or invalid, a exception should be thrown.
*/

FORMAT_API void fmtPrint(Context& ctx, const char* data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, char data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, signed char data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed short data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed int data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed long data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed long long data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, unsigned char data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned short data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned int data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned long data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned long long data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, const void* data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, float data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, double data, const Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, long double data, const Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, bool data, const Placeholder& placeholder);
struct Cursor
{
	int pos;
};
FORMAT_API void fmtPrint(Context& ctx, Cursor* ptr, const Placeholder& placeholder);

}
#endif // #ifndef INCLUDED_FORMAT_CONVERTERS_H
