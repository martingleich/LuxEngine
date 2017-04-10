#pragma once
#include <cassert>
#include "StringBasics.h"
#include "Context.h"
#include "Locale.h"
#include "Placeholder.h"

namespace format
{

slice ConvertString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);
slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);
slice* CopyConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize);
slice* ConvertAddString(Context& dst, StringType srcType, const char* srcData, size_t srcSize, const Cursor& curDiff);

namespace internal
{
	void reverse(char* s, size_t len);

	// sign pre.postEexp
	// MAX_INT_LEN + 1 + digits + 1 + MAX_INT_LEN
	// 11 + 1 + 11 + 1= 24
	static size_t MAX_DOUBLE_LEN = 24;

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

	//! Convert a floating point number to a string.
	/**
	for Not a number nan is written
	for infinite +inf or -inf is written.
	For number bigger than 10^13 or smaller than 10^-9 expontial notation is used in format 12.34e+5
	\param n The number to convert
	\param [out] s Here the resulting string is written. Must be at least MAX_DOUBLE_LEN+digits character long, String is not NULL-terminated.
	\param digiti The maximal number of digits after the decimal-point.
	\return The length of the resulting string, 0 if an error occured.
	*/
	void ftoa(Context& ctx, double n, int digits, bool forcePrecision, const locale::Facet_NumericalFormat& locale);

	void hftoa(Context& ctx, double n, const locale::Facet_NumericalFormat& locale);

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

	void PutCount(Context& ctx, size_t count, StringType type, const char* buffer, size_t maxCount);
	void PutSpaces(Context& ctx, size_t count);
	void FormatTilde(Context& ctx, const Placeholder& placeholder);
	void FormatTab(Context& ctx, const Placeholder& placeholder);
	bool TryFormatArgFree(Context& ctx, Placeholder& placeholder);
}

}