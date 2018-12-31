#ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
#define INCLUDED_FORMAT_CONV_INTERNAL_H
#include "format/Context.h"
#include "format/FormatLocale.h"
#include <cassert>

namespace format
{

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
FORMAT_API void PutFloat(Context& ctx, double n, int digits, bool forcePrecision, const Facet_NumericalFormat& locale);

FORMAT_API void PutHexFloat(Context& ctx, double n, const Facet_NumericalFormat& locale);

FORMAT_API void PutInt(Context& ctx, unsigned int i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale);
FORMAT_API void PutLong(Context& ctx, unsigned long i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale);
FORMAT_API void PutLongLong(Context& ctx, unsigned long long i, bool sign, bool forceSign, int precision, int base, bool prefix, const Facet_NumericalFormat& locale);

//! Put any number of characters of a given type into a context
/**
Putting is optimized if a bigger number of characters is given.
\param ctx The context where the characters are written to.
\param charSize The size of a character in bytes.
\param count The number of characters to write
\param buffer A buffer of characters to put, the buffer must contain copies of the same character.
\param bufferSize The number of bytes in the buffer
*/
FORMAT_API void PutCount(Context& ctx, int charSize, int count, const char* buffer, int bufferSize);

//! Put any number of spaces into the output
FORMAT_API void PutSpaces(Context& ctx, int count);

//! Put any number of zeros into the output
FORMAT_API void PutZeros(Context& ctx, int count);

////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////

namespace parser
{
struct BasicPlaceholder
{
public:
	class Option
	{
	public:
		Option() : value(-1) {}
		explicit Option(int i) : value(i) {}
		bool IsEnabled() const { return value != -1; }
		bool IsDefault() const { return IsEnabled() && !HasValue(); }
		bool HasValue() const { return value >= 0; }
		int GetValue(int default) const { if(HasValue()) return value; else return default; }

	private:
		int value;
	};

public:
	FORMAT_API static BasicPlaceholder Parse(Slice slice, Context& ctx, int baseArgId);

public:
	Option dot;
	Option star;
	Option hash;
	Option plus;
	Option at;
};
}
}

#endif // #ifndef INCLUDED_FORMAT_CONV_INTERNAL_H
