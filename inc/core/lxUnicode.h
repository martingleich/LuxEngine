#ifndef INCLUDED_LUX_UNICODE_H
#define INCLUDED_LUX_UNICODE_H
#include "core/LuxBase.h"
#include "core/iterators/lxBaseIterator.h"

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
LUX_API int StringLengthUTF8(const char* str, int* outBytes = nullptr);

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
LUX_API int StringLengthUTF16(const char* str, int* outBytes = nullptr);

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

//! Iterator over the codepoints of the string.
class ConstUTF8Iterator : public core::BaseIterator<core::BidirectionalIteratorTag, u32>
{
public:
	//! Creates a invalid iterator.
	ConstUTF8Iterator() :
		m_Data(nullptr),
		m_First(nullptr)
	{
	}
	//! Create a iterator.
	/**
	\param ptr The character reference by the iterator.
	\param first The first character of the string referenced.
	*/
	ConstUTF8Iterator(const char* ptr, const char* first) :
		m_Data(ptr),
		m_First(first)
	{
	}
	ConstUTF8Iterator(const char* ptr) :
		m_Data(ptr),
		m_First(nullptr)
	{
	}

	static ConstUTF8Iterator Invalid()
	{
		return ConstUTF8Iterator();
	}

	operator const char*()
	{
		return m_Data;
	}

	ConstUTF8Iterator& operator++()
	{
		if(m_Data < m_First)
			++m_Data;
		else
			core::AdvanceCursorUTF8(m_Data);
		return *this;
	}
	ConstUTF8Iterator& operator--()
	{
		if(m_Data <= m_First)
			--m_Data;
		else
			core::RetractCursorUTF8(m_Data);
		return *this;
	}
	ConstUTF8Iterator operator++(int)
	{
		ConstUTF8Iterator tmp(*this);
		++*this;
		return tmp;
	}

	ConstUTF8Iterator operator--(int)
	{
		ConstUTF8Iterator tmp(*this);
		++*this;
		return tmp;
	}

	ConstUTF8Iterator Next()
	{
		ConstUTF8Iterator tmp(*this);
		++tmp;
		return tmp;
	}

	ConstUTF8Iterator Prev()
	{
		ConstUTF8Iterator tmp(*this);
		--tmp;
		return tmp;
	}

	ConstUTF8Iterator& operator+=(intptr_t i)
	{
		if(i < 0)
			*this -= -i;
		while(i--)
			++*this;
		return *this;
	}

	ConstUTF8Iterator& operator-=(intptr_t i)
	{
		if(i < 0)
			*this += -i;
		while(i--)
			--*this;
		return *this;
	}

	ConstUTF8Iterator operator+(intptr_t i)
	{
		ConstUTF8Iterator out(*this);
		out += i;
		return out;
	}

	ConstUTF8Iterator operator-(intptr_t i)
	{
		ConstUTF8Iterator out(*this);
		out -= i;
		return out;
	}

	bool operator==(ConstUTF8Iterator other) const
	{
		return m_Data == other.m_Data;
	}
	bool operator!=(ConstUTF8Iterator other) const
	{
		return !(*this == other);
	}
	bool operator<(ConstUTF8Iterator other) const
	{
		return m_Data < other.m_Data;
	}

	bool operator<=(ConstUTF8Iterator other) const
	{
		return m_Data <= other.m_Data;
	}

	bool operator>(ConstUTF8Iterator other) const
	{
		return m_Data > other.m_Data;
	}

	bool operator>=(ConstUTF8Iterator other) const
	{
		return m_Data >= other.m_Data;
	}

	u32 operator*() const
	{
		return core::GetCharacterUTF8(m_Data);
	}

	//! Access the character the iterator is referencing
	/**
	All valid iterators point to a continous block of memory.
	\return Pointer to the character referenced by the iterator.
	*/
	const char* Pointer() const
	{
		return m_Data;
	}
	
	const char* First() const
	{
		return m_First;
	}

private:
	const char* m_Data; //!< The character referenced by the iterator, always the first element of a utf8 sequence if valid.
	const char* m_First; //!< The first character of the string containing the iterator.
};

}
}
#endif // #ifndef INCLUDED_LUX_UNICODE_H
