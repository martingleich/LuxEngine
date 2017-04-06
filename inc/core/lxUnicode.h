#ifndef INCLUDED_LX_UNICODE_H
#define INCLUDED_LX_UNICODE_H
#include "LuxBase.h"

namespace lux
{
namespace core
{

//! Computes the number of codepoints inside a utf8 string.
/**
The passed string must be nul-terminated.
\param str The string data.
\return The number of characters in the string without NUL.
*/
LUX_API size_t StringLengthUTF8(const char* str);

//! Moves the passed utf8 cursor onto the character before the current one
LUX_API void RetractCursorUTF8(const char*& ptr);

//! Moves the passed utf8 cursor onto the character after the current one
/**
\param [inout] The utf8-cursor.
\return The value of the current character.
*/
LUX_API u32 AdvanceCursorUTF8(const char*& ptr);

//! Read the current character from a utf8-string.
/**
\return The value of the current character.
*/
LUX_API u32 GetCharacterUTF8(const char* ptr);


//! Computes the number of codepoints inside a utf16 string.
/**
The passed string must be nul-terminated.
\param str The string data.
\return The number of characters in the string without NUL.
*/
LUX_API size_t StringLengthUTF16(const char* str);

//! Moves the passed utf16 cursor onto the character after the current one
/**
\param [inout] The utf16-cursor.
\return The value of the current character.
*/
LUX_API u32 AdvanceCursorUTF16(const char*& ptr);

//! Read the current character from a utf16-string.
/**
\return The value of the current character.
*/
LUX_API u32 GetCharacterUTF16(const char* ptr);

//! Compares two unicode code-points case insensitive.
LUX_API bool IsEqualCaseInsensitive(u32 a, u32 b);

}
}
#endif // #ifndef INCLUDED_LX_UNICODE_H