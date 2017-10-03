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
\param [out] outBytes If not null the number of bytes in the string is written here
\return The number of characters in the string without NUL.
*/
LUX_API size_t StringLengthUTF8(const char* str, size_t* outBytes=nullptr);

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
\param [out] outBytes If not null the number of bytes in the string is written here
\return The number of characters in the string without NUL.
*/
LUX_API size_t StringLengthUTF16(const char* str, size_t* outBytes=nullptr);

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

//! Convert to lower-case
LUX_API u32 ToLowerChar(u32 c);

//! Convert to upper-case
LUX_API u32 ToUpperChar(u32 c);

//! Is the character a digit
LUX_API bool IsDigit(u32 c);

//! Is the character invisible if printed
LUX_API bool IsSpace(u32 c);

//! Is the character a letter
LUX_API bool IsAlpha(u32 c);

//! Is the character in upper-case
LUX_API bool IsUpper(u32 c);

//! Is the character in lower-case
LUX_API bool IsLower(u32 c);

enum class EUnicodeClass
{
	Other,
	LetterUpper,
	LetterLower,
	LetterTitel,
	LetterModifier,
	LetterOther,
	MarkNonSpacing,
	MarkSpacingCombining,
	MarkEnclosing,
	NumberDecimalDigit,
	NumberLetter,
	NumberOther,
	PunctuationConnector,
	PunctuationDash,
	PunctuationOpen,
	PunctuationClose,
	PunctuationInitialQuote,
	PunctuationFinalQuote,
	PunctuationOther,
	SymbolMath,
	SymbolCurrency,
	SymbolModifier,
	SymbolOther,
	SeperatorSpace,
	SeperatorLine,
	SeperatorParagraph,
	OtherControl,
	OtherFormat,
	OtherSurrogate,
	OtherPrivateUse,
};

LUX_API EUnicodeClass CategorizeCodePoint(u32 c);

}
}
#endif // #ifndef INCLUDED_LX_UNICODE_H