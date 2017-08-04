#ifndef INCLUDED_FORMAT_STRING_TYPE_H
#define INCLUDED_FORMAT_STRING_TYPE_H

#include <stddef.h>

namespace format
{
//! The diffrent types supported by the library
/**
Not all type conversions are supported every free.
It's best to stick only to utf8, or only to ASCII.
Conversions from utf8 oder codepoints to ASCII, are lossy
*/
enum StringType
{
	Ascii,     //!< 7-Bit ASCII
	Unicode,   //!< UTF-8
	CodePoint, //!< 32-Bit Unicode Codepoint
};

//! Get the number of bytes for single character of a given type
/**
Returns 0 if the number of bytes is not fixed or not valid
*/
inline size_t GetBytePerChar(StringType t)
{
	switch(t) {
	case Unicode:
		return 0;
	case Ascii:
		return 1;
	case CodePoint:
		return 4;
	default:
		return 0;
	}
}
}
#endif // #ifndef INCLUDED_FORMAT_STRING_TYPE_H
