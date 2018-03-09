#ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
#define INCLUDED_FORMAT_CONV_INTERNAL_H
#include "format/Context.h"
#include "format/FormatLocale.h"

namespace format
{
//! Reverse an ASCII string
FORMAT_API void reverse(char* s, size_t len);

// sign pre.postEexp
// MAX_INT_LEN + 1 + digits + 1 + MAX_INT_LEN
// 11 + 1 + 11 + 1= 24
//static size_t MAX_DOUBLE_LEN = 24;

//! Convert a unsigned interger to an ASCII string
template <typename T>
size_t uitoa(T n, char* s, int base)
{
	if(base > 10 + 26 || base < 2)
		return 0;

	char* c = s;
	// Record digits in inverse order
	do {
		char digit = (char)(n % base);
		*c++ = digit + (digit < 10 ? '0' : ('A' - 10));
	} while((n /= base) > 0);

	// Reverse the rest.
	reverse(s, c - s);

	return c - s;
}

// The maximum number of characters used by this function
// are 42 byte
// sign + pre + comma + post
// 1 + ceil(log10(2^64)) + 1 + ceil(log10(2^64))
FORMAT_API size_t ftoaSimple(double n, int digits, char* str);

//! Convert a floating point number to a string.
/**
for Not a number nan is written
for infinite +inf or -inf is written.
For number bigger than 10^13 or smaller than 10^-9 expontial notation is used in format 12.34e+5
\param n The number to convert
\param [out] s Here the resulting string is written. Must be at least MAX_DOUBLE_LEN+digits character long, String is not nullptr-terminated.
\param digiti The maximal number of digits after the decimal-point.
\return The length of the resulting string, 0 if an error occured.
*/
FORMAT_API void ftoa(Context& ctx, double n, int digits, bool forcePrecision, const Facet_NumericalFormat& locale);

FORMAT_API void hftoa(Context& ctx, double n, const Facet_NumericalFormat& locale);

//! Convert a character to it's long representation
/**
A characters long represenation is the normal character for simple letters digits or signs.
For special or invisible character like space, backspace, null or linefeed it's a describing string
(i.e. <space>, <backspace>, <null>, <linefeed>). <br>
Allocates memory from the Context.
\param ctx The format context, no visible changes are made to it.
\param c The character to convert
\return A slice for the long form.
*/
FORMAT_API void AddCharLong(Context& ctx, uint32_t c);

//! Put any number of characters of a given type into a context
/**
Putting is optimized if a bigger number of characters is given.
\param count The number of characters to write
\param buffer A buffer of characters to put, the buffer must contain copies of the same character.
\param bufferSize The number of bytes in the buffer
*/
FORMAT_API void PutCount(Context& ctx, size_t count, const char* buffer, size_t bufferSize);

//! Put any number of spaces into the output
FORMAT_API void PutSpaces(Context& ctx, size_t count);

//! Put any number of spaces into the output
FORMAT_API void PutZeros(Context& ctx, size_t count);

}

#endif // #ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
