#ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
#define INCLUDED_FORMAT_CONV_INTERNAL_H
#include "core/LuxBase.h"
#include "format/StringBasics.h"
#include "format/Context.h"
#include "format/FormatLocale.h"
#include "format/Placeholder.h"

namespace format
{

//! Convert a given string to a format compatible with a context
/**
Warning: Maybe the new slice points to the same data which was passed.
*/
LUX_API Slice ConvertString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);
//! Convert a given string to a format compatible with a context and add it to the context
/**
Warning: Maybe the new slice points to the same data which was passed.
\return A pointer to the newly created string
*/
LUX_API Slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);

//! Convert a given string to a format compatible with a context and add it to the context
/**
Forces the creation of a newly created string, srcData can be changed after the call without effect
\return A pointer to the newly created string
*/
LUX_API Slice* CopyConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);

//! Convert a given string to a format compatible with a context and add it to the context
/**
Warning: Maybe the new slice points to the same data which was passed.
\param curDiff The amount the cursor changes, when adding the slice, is automaticly updated in the other versions
\return A pointer to the newly created string
*/
LUX_API Slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize, const Cursor& curDiff);

namespace internal
{
	//! Reverse an ASCII string
	void reverse(char* s, size_t len);

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
	size_t ftoaSimple(double n, int digits, char* str);

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
	void ftoa(Context& ctx, double n, int digits, bool forcePrecision, const Facet_NumericalFormat& locale);

	void hftoa(Context& ctx, double n, const Facet_NumericalFormat& locale);

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
	void AddCharLong(Context& ctx, uint32_t c);

	//! Put any number of characters of a given type into a context
	/**
	Putting is optimized if a bigger number of characters is given
	\param count The number of characters to write
	\param type The strign type of the passed buffer
	\param buffer A buffer of characters to put
	\param maxCount The number of characters in the buffer
	*/
	void PutCount(Context& ctx, size_t count, StringType type, const char* buffer, size_t maxCount);

	//! Put any number of spaces into the output
	void PutSpaces(Context& ctx, size_t count);

	//! Format and write a tilde placeholder to the output
	void FormatTilde(Context& ctx, const Placeholder& placeholder);

	//! Format and write a tab placeholder to the output
	void FormatTab(Context& ctx, const Placeholder& placeholder);

	//! Try to format a argument-free placeholder
	/**
	\return True if there was an argument-free placeholder to replace, otherwise false
	*/
	bool TryFormatArgFree(Context& ctx, Placeholder& placeholder);
}

}

#endif // #ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
