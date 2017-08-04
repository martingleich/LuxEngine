#ifndef INCLUDED_FORMAT_CONVERTERS_H
#define INCLUDED_FORMAT_CONVERTERS_H
#include "Context.h"
#include "Placeholder.h"
#include "Exception.h"
#include "StringBasics.h"

namespace format
{

/*
The conv_data function is called to convert a placeholder-argument to a string.
Each conv_data function, must convert it's argument to a string and
add it to the Context.
There can be additional memory allocation.
If a conversion is not possible or invalid, a exception should be thrown.
The passed FormatOptions can be changed to change the behavior of the align operation
following the conv_data call.
*/

LUX_API void conv_data(Context& ctx, const char* data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, intmax_t data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, uintmax_t data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, const void* data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, double data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, bool data, Placeholder& placeholder);
LUX_API void conv_data(Context& ctx, Cursor* ptr, Placeholder& placeholder);

//! Convert a signed integer to a string
/**
The resulting string will be null terminated
\param data The integer to convert
\param str Here the new string is written, must be big enough to hold log(data)/log(base) + 2 bytes
This is _not_ checked inside the function
\param base The base to convert into
\return The number of bytes written, without the terminating null
*/
LUX_API size_t IntToString(intmax_t data, char* str, int base = 10);

//! Convert a unsigned integer to a string
/**
The resulting string will be null terminated
\param data The integer to convert
\param str Here the new string is written, must be big enough to hold log(data)/log(base) + 1 bytes
This is _not_ checked inside the function
\param base The base to convert into
\return The number of bytes written, without the terminating null
*/
LUX_API size_t UIntToString(uintmax_t data, char* str, int base = 10);

//! Convert a floating point number to a string
/**
The resulting string will be null terminated.
Zero will be written as "0".
Infinitiy will be written as "inf".
Not-A-Number will be written as "nan".
Trailing zeros will never be written.
\param data The number to convert
\param str Here the new string is written, must be big enough to hold 43 bytes
This is _not_ checked inside the function
\param precision The requested number of digits behind the comma, must be between 1 and 10, numbers outside are clamped
\return The number of bytes written, without the terminating null
*/
LUX_API size_t FloatToString(double data, char* str, int precision = 3);

}
#endif // #ifndef INCLUDED_FORMAT_CONVERTERS_H
