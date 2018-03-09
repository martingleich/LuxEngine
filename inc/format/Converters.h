#ifndef INCLUDED_FORMAT_CONVERTERS_H
#define INCLUDED_FORMAT_CONVERTERS_H
#include "format/Context.h"
#include "format/Placeholder.h"

namespace format
{

/*
The fmtPrint function is called to convert a placeholder-argument to a string.
Each fmtPrint function, must convert it's argument to a string and
add it to the Context.
There can be additional memory allocation.
If a conversion is not possible or invalid, a exception should be thrown.
The passed FormatOptions can be changed to change the behavior of the align operation
following the fmtPrint call.
*/

FORMAT_API void fmtPrint(Context& ctx, const char* data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, char data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, signed char data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed short data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed int data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed long data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, signed long long data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, unsigned char data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned short data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned int data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned long data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, unsigned long long data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, const void* data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, float data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, double data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, long double data, Placeholder& placeholder);

FORMAT_API void fmtPrint(Context& ctx, bool data, Placeholder& placeholder);
FORMAT_API void fmtPrint(Context& ctx, Cursor* ptr, Placeholder& placeholder);

//! Convert a signed integer to a string
/**
The resulting string will be null terminated
\param data The integer to convert
\param str Here the new string is written, must be big enough to hold log(data)/log(base) + 2 bytes
This is _not_ checked inside the function
\param base The base to convert into
\return The number of bytes written, without the terminating null
*/
FORMAT_API size_t IntToString(intmax_t data, char* str, int base = 10);

//! Convert a unsigned integer to a string
/**
The resulting string will be null terminated
\param data The integer to convert
\param str Here the new string is written, must be big enough to hold log(data)/log(base) + 1 bytes
This is _not_ checked inside the function
\param base The base to convert into
\return The number of bytes written, without the terminating null
*/
FORMAT_API size_t UIntToString(uintmax_t data, char* str, int base = 10);

//! Convert a floating point number to a string
/**
The resulting string will be null terminated.
Zero will be written as "0".
Infinity will be written as "inf".
Not-A-Number will be written as "nan".
Trailing zeros will never be written.
\param data The number to convert
\param str Here the new string is written, must be big enough to hold 43 bytes
This is _not_ checked inside the function
\param precision The requested number of digits behind the comma, must be between 1 and 10, numbers outside are clamped
\return The number of bytes written, without the terminating null
*/
FORMAT_API size_t FloatToString(double data, char* str, int precision = 3);

}
#endif // #ifndef INCLUDED_FORMAT_CONVERTERS_H
