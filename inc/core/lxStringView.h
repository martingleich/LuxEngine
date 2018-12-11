#ifndef INCLUDED_LUX_STRING_VIEW_H
#define INCLUDED_LUX_STRING_VIEW_H
#include "core/LuxBase.h"
#include "core/lxIterator.h"
#include "core/lxUnicode.h"

namespace lux
{
namespace core
{
//! The type of characters a string contains.
enum class EStringClassFlag
{
	Digit = 1,      //!< The string only contains digits.
	Alpha = 2,      //!< The string only contains letters.
	AlphaNum = 4,   //!< The string only contains digits and letters.
	Space = 8,      //!< The string only contains whitespace.
	Upper = 16,     //!< All the characters in the string are upper-case.
	Lower = 32,     //!< All the characters in the string are lower-case.
	Empty = 64,     //!< The string was empty
};

enum class EStringCompare
{
	CaseSensitive,
	CaseInsensitive,
};

//! Dummy string type
/**
Alias type for full strings or character-arrays.
Contains UTF8-Data.
Can be used to speed up a function receiving character pointers and strings.
Warning may not be null-terminated.
*/
class StringView
{
public:
	LUX_API static const StringView EMPTY;

	//! Create a empty string view.
	constexpr StringView() :
		m_Size(0),
		m_Data("")
	{
	}

	//! Create a string view from a literal string.
	template <size_t N>
	constexpr StringView(const char (&str)[N]) :
		m_Size(N-1),
		m_Data(str)
	{
	}

	//! Create a dummy string from a pointer
	/**
	\param str The data of the string.
	\param s The number of bytes in the string.
	*/
	constexpr StringView(const char* str, int s) :
		m_Size(s),
		m_Data(str)
	{
		lxAssert(str && s >= 0);
	}

	StringView(const StringView&) = default;

	//! The size of the string.
	int Size() const { return m_Size; }

	//! Access the bytes of the string.
	const char* Data() const { return m_Data; }

	//! Access a byte of the string.
	char operator[](int i) const
	{
		lxAssert(i >= 0 && i < m_Size);
		return m_Data[i];
	}

	//! Get a substring of the string.
	/**
	\param first The index of the first element of the substring.
	\param size The number of bytes in the substring.
	\return A stringview containg the elements [first, first+size)
	*/
	StringView SubString(int first, int size) const
	{
		lxAssert(first >= 0 && size >= 0 && first+size <= m_Size);
		return StringView(m_Data + first, size);
	}

	//! Get a substring of the string.
	/**
	Get a substring to the end of the string.
	\param first The index of the first element of the substring.
	\return A stringview containg the elements [first, size)
	*/
	StringView EndSubString(int first) const { return SubString(first, m_Size-first); }

	//! Get a substring of the string.
	/**
	Get a substring from the begining of the string.
	\param last The index of the last element of the substring.
	\return A stringview containg the elements [0, last)
	*/
	StringView BeginSubString(int last) const { return SubString(0, last); }

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	LUX_API bool Equal(const StringView& other, EStringCompare = EStringCompare::CaseSensitive) const;

	//! Compare two strings.
	LUX_API bool Smaller(const StringView& other, EStringCompare = EStringCompare::CaseSensitive) const;

	//! Test if the string starts with a given string.
	/**
	\param data The string to test with.
	\return True, if this string starts with the given one, false otherwise
	*/
	LUX_API bool StartsWith(const StringView& data, EStringCompare = EStringCompare::CaseSensitive) const;

	//! Test if the string ends with a given string.
	/**
	\param data The string to test with.
	\return True, if this string starts with the given one, false otherwise
	*/
	LUX_API bool EndsWith(const StringView& data, EStringCompare = EStringCompare::CaseSensitive) const;

	//! Find the first occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is always found.
	\return The index of the first occurene or -1.
	*/
	LUX_API int Find(const StringView& search) const;

	//! Find the last occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is always found.
	\return The index of the last occurene or -1.
	*/
	LUX_API int FindReverse(const StringView& search) const;

	//! Classify the content of the string
	/**
	See \ref{EStringType} for more information about string classification.
	*/
	LUX_API EStringClassFlag Classify() const;

	//! Contains the string only whitespace(or is empty)
	LUX_API bool IsWhitespace() const;

	//! Is the string empty
	bool IsEmpty() const { return (m_Size == 0); }

	//! The bytes of the string as range.
	core::Range<const char*> Bytes() const
	{
		return MakeRange<const char*>(m_Data, m_Data + m_Size);
	}
	//! The unicode codepoints of the string as range.
	core::Range<ConstUTF8Iterator> CodePoints() const
	{
		return MakeRange<ConstUTF8Iterator>(m_Data, m_Data + m_Size);
	}

	/*
	Set maxCount to a negative number to ignore maxCount
	*/
	template <typename AddResultT>
	int BasicSplit(StringView split, int maxCount, bool ignoreEmpty, AddResultT&& outputter)
	{
		if(maxCount == 0) == 0)
			return 0;

		int count = 0;
		const char* inCur = this->Data();
		const char* inEnd = inCur + this->Size();
		const char* splitCur = inCur;
		int splitSize = 0;
		if(split.Size() == 0) {
			while(inCur != inEnd) {
				if(count == maxCount)
					break;
				outputter(StringView(inCur, 1));
				++count;
				++inCur;
			}
			return count;
		} else if(split.Size() == 1) {
			while(inCur+1 <= inEnd) {
				if(*inCur == *split.Data()) {
					if(!(ignoreEmpty && splitSize == 0)) {
						outputter(StringView(splitCur, splitSize));
						++count;
					}
					++inCur;
					splitCur = inCur;
					splitSize = 0;
					if(count == maxCount)
						break;
				} else {
					++splitSize;
					++inCur;
				}
			}
		} else {
			while(inCur+split.Size() <= inEnd) {
				if(std::memcmp(inCur, split.Data(), split.Size()) == 0) {
					if(!(ignoreEmpty && splitSize == 0)) {
						outputter(StringView(splitCur, splitSize));
						++count;
						if(count == maxCount)
							break;
					}
					splitCur = inCur + split.Size();
					splitSize = 0;
				} else {
					++splitSize;
					++inCur;
				}
			}
		}

		if(!(ignoreEmpty && splitSize == 0) && count != maxCount) {
			outputter(StringView(splitCur, splitSize));
			++count;
		}
		return count;
	}

private:
	//! The number of bytes in the string, without the NUL-Byte.
	int m_Size;

	//! Pointer to the string-data
	/**
	Nul-Terminated c-string.
	*/
	const char* m_Data;
};

template <int MAX_SIZE>
class FixedString
{
public:
	FixedString()
	{
		m_Size = 0;
	}
	FixedString(const char* str, int size)
	{
		lxAssert(size <= MAX_SIZE);
		std::memcpy(m_Data, str, size);
		m_Size = size;
	}
	FixedString(const FixedString& other) = default;
	FixedString& operator=(const FixedString& other) = default;

	StringView AsView() const { return StringView(m_Data, m_Size); }

private:
	char m_Data[MAX_SIZE];
	int m_Size;
};

}
}

#endif // #ifndef INCLUDED_LUX_STRING_VIEW_H
