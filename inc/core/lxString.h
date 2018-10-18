#ifndef INCLUDED_LUX_STRING_H
#define INCLUDED_LUX_STRING_H
#include "core/lxMemory.h"
#include "core/lxUnicode.h"
#include "core/lxUtil.h"
#include "core/lxIterator.h"
#include "core/lxException.h"
#include "core/lxTypes.h"
#include "core/lxStringView.h"

namespace lux
{
namespace core
{
template <typename T> 
class Array;

//! A utf8-string
/**
All non-constant methods invalidate all iterators.
No string can contain the NUL.
These strings are always nullterminated
Length or Count refers to a number of codepoints.
Size refers to number of bytes.
*/
class String
{
public:
	//! The empty string.
	/**
	Can be used to pass referenced without creating a temporary.
	*/
	LUX_API static const String EMPTY;

public:
	//! Creates a empty string.
	LUX_API String();
	//! Create a string from a c-string.
	/**
	\param data A pointer to nul-terminated string data, if null a empty string is created.
	\param size The number of bytes to copy from the string if -1 all bytes a copied.
	*/
	LUX_API String(const char* data, int size = -1);
	String(StringView view) :
		String(view.Data(), view.Size())
	{
	}

	//! Copyconstructor
	LUX_API String(const String& other);
	//! Moveconstructor
	LUX_API String(String&& old);

	//! Destructor
	LUX_API ~String();

	//! Creates a copy of this string.
	LUX_API String Copy();

	StringView AsView() const { return StringView(Data(), Size()); }
	operator StringView() const { return AsView(); }

	//! Reserve size bytes for string memory.
	/**
	After this call Data() will point to size+1 bytes of memory.
	\param size The number of bytes to allocate for string-storage without terminating NUL.
	*/
	LUX_API void Reserve(int size);

	//! Copy-assignment
	LUX_API String& operator=(const String& other);
	//! Copy-assignment
	LUX_API String& operator=(const char* other);
	//! Move-assignment
	LUX_API String& operator=(String&& old);

	//! Append another string
	String& operator+=(StringView str) { return Append(str); }

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	bool Equal(StringView other, EStringCompare compare = EStringCompare::CaseSensitive) const
	{
		return ((StringView)*this).Equal(other, compare);
	}

	//! Compare two strings.
	bool Smaller(StringView other, EStringCompare compare = EStringCompare::CaseSensitive) const
	{
		return ((StringView)*this).Smaller(other, compare);
	}

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted codepoint.
	\param other The string to insert.
	*/
	LUX_API void Insert(int pos, StringView other);
	LUX_API void InsertCodePoint(int pos, u32 value);

	//! Append another string onto this one.
	/**
	\param other The string to append.
	\return selfreference
	*/
	String& Append(StringView other) { Insert(Size(), other); return *this; }
	String& Append(const char* data, int size) { return Append(StringView(data, size)); }

	//! Append a single codepoint from a iterator.
	/**
	\param codepoint The codepoint referenced by this iterator is appended.
	\return selfreference
	*/
	LUX_API String& AppendCodePoint(const void* codepoint);

	//! Append a single codepoint.
	/**
	\param codepoint A unicode-codepoint to append
	\return selfreference
	*/
	String& AppendCodePoint(u32 codepoint) { InsertCodePoint(Size(), codepoint); return *this; }

	//! Resize this string
	/**
	If the new size is smaller of equal to the current size, the end of the string
	is cut away so it's new size is achived.
	If the new size is bigger than the current size, bytes are read from filler
	and added to the string until the new size is reached, if more bytes are needed than filler contains,
	bytes are read from the start again.
	\param newSize The new size of the string.
	\param filler The string to fill the newly created string with
	*/
	LUX_API void Resize(int newSize, StringView filler = " ");

	//! Clear the string contents, making the string empty.
	LUX_API String& Clear();

	//! The number of bytes contained in the string, without NUL.
	inline int Size() const
	{
		return m_Size;
	}

	//! The number of codepoints in the string.
	inline int CodePointCount() const
	{
		return CodePoints().End() - CodePoints().First();
	}

	//! The number of bytes allocated for the string.
	inline int Allocated() const
	{
		return m_Allocated - 1;
	}

	//! Is the string empty.
	/*
	The string is empty if is size and length are zero.
	*/
	inline bool IsEmpty() const
	{
		return (Size() == 0);
	}

	//! Gives access to the raw string-data.
	/**
	A pointer to at least Size()+1 bytes of string data.
	*/
	LUX_API const char* Data() const;

	//! Gives access to the raw string-data.
	/**
	A pointer to at least Size()+1 bytes of string data.
	Can be used to manipulate the string, use with care.
	*/
	LUX_API char* Data();

	//! Add a single utf-8 byte to the string.
	LUX_API void PushByte(u8 byte);

	inline core::Range<ConstUTF8Iterator> CodePoints() const
	{
		return MakeRange<ConstUTF8Iterator>(Data(), Data() + Size());
	}
	inline core::Range<const char*> Bytes() const
	{
		return MakeRange<const char*>(Data(), Data() + Size());
	}
	inline core::Range<char*> Bytes()
	{
		return MakeRange<char*>(Data(), Data() + Size());
	}

	char operator[](int i) const
	{
		lxAssert(i >= 0 && i < Size());
		return Data()[i];
	}
	char& operator[](int i)
	{
		lxAssert(i >= 0 && i < Size());
		return Data()[i];
	}

	//! Test if the string starts with a given string.
	/**
	\param data The string to test with.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool StartsWith(StringView data, EStringCompare cmp = EStringCompare::CaseSensitive) const
	{
		return ((StringView)*this).StartsWith(data, cmp);
	}

	//! Test if the string ends with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the End() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool EndsWith(StringView data, EStringCompare cmp = EStringCompare::CaseSensitive) const
	{
		return ((StringView)*this).EndsWith(data, cmp);
	}

	//! Replace all occurences of a substring in this string.
	/**
	This method doesn't work recursive.
	\param replace The string which the search string is replaced with.
	\param search The string to search for, my not be empty.
	\param first The iterator from where to start the search, if invalid First() is used.
	\param end The iterator where the search is stopped, if invalid End() is used.
	\return The number of occurences found and replaced.
	*/
	LUX_API int Replace(StringView replace, StringView search, int first = -1, int size = -1);

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with.
	\param rangeFirst The first iterator of the replaced range.
	\param size The number of bytes to replace.
	\return A iterator to the first character after the newly inserted string.
	*/
	LUX_API int ReplaceRange(StringView replace, int rangeFirst, int size = -1);

	//! Find the first occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	int Find(StringView search) const
	{
		return ((StringView)*this).Find(search);
	}

	//! Find the last occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	int FindReverse(StringView search) const
	{
		return ((StringView)*this).FindReverse(search);
	}

	String SubString(int first, int size) const { return SubStringView(first, size); }
	String EndSubString(int begin) const { return EndSubStringView(begin); }
	String BeginSubString(int end) const { return BeginSubStringView(end); }

	StringView SubStringView(int first, int size) const { return ((StringView)(*this)).SubString(first, size); }
	StringView EndSubStringView(int begin) const { return SubStringView(begin, Size()-begin); }
	StringView BeginSubStringView(int end) const { return SubStringView(0, end); }

	//! Removes a number of characters from the back of string.
	/**
	\param size The number of characters to bytes, may be bigger than the number of bytes in the string.
	\return The actual number of bytes removed, if not equal to count, the string will be empty.
	*/
	LUX_API int Pop(int size = 1);

	//! Removes bytes from the string.
	/**
	\param pos Where should the byte be removed.
	\param size The number of bytes to remove.
	*/
	LUX_API void Remove(int pos, int size);

	//! Removes all whitespace from the right side of the string.
	/**
	Codepoints are removed from the back of the string, until a not space character is encountered.
	\param end Where should the removing of characters start, if invalid End() is used.
	\return selfreference
	*/
	LUX_API String& RStrip(int end = -1);

	//! Removes all whitespace from the left side of the string.
	/**
	Codepoints are removed from the front of the string, until a not space character is encountered.
	\param first Where should the removing of characters start, if invalid First() is used.
	\return selfreference
	*/
	LUX_API String& LStrip(int first = -1);

	//! Removes all whitespace from the left and right side of the string.
	/**
	Codepoints are removed from either side of the string, until a not space character is encountered.
	\param first Where should the removing of characters start, if invalid First() is used.
	\param end Where should the removing of characters start, if invalid End() is used.
	\return selfreference
	*/
	LUX_API String& Strip(int first = -1, int size = -1);

	// TODO: Replace split character with string.
	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\param outArray Where to write the substrings.
	\param maxCount The amount of strings available in the output array.
	\param ignoreEmpty Empty split strings aren't added to the output
	\return The number of written output strings.
	*/
	LUX_API int Split(StringView split, String* outArray, int maxCount, bool ignoreEmpty = false) const;

	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\param ignoreEmpty Empty split strings aren't added to the output
	\return The array of substrings
	*/
	LUX_API core::Array<String> Split(StringView split, bool ignoreEmpty = false) const;

	//! Classify the content of the string
	/**
	See \ref{EStringType} for more information about string classification.
	*/
	EStringClassFlag Classify() const
	{
		return ((StringView)*this).Classify();
	}

	//! Contains the string only whitespace(or is empty)
	bool IsWhitespace() const
	{
		return ((StringView)*this).IsWhitespace();
	}

	//! Get a string in lower case
	LUX_API String GetLower() const;

	//! Get a string in upper case
	LUX_API String GetUpper() const;

private:
	//! Is the string saved in short format.
	LUX_API bool IsShortString() const;

	//! Returns the number of allocated bytes, including NUL.
	LUX_API int GetAllocated() const;

	//! Set the number of allocated bytes, including NUL.
	LUX_API void SetAllocated(int a);

private:
	//! Contains the raw data of the string.
	/**
	Never access this member always used the Data() function.
	The string is saved in utf-8 format and is null-terminated.
	Will be null if the string is empty.
	*/
	union DataUnion
	{
		char* ptr;
		char raw[16];
	} m_Data;

	static const int MAX_SHORT_BYTES = sizeof(DataUnion);

	// The number of bytes allocated for the string, including the terminating NULL character.
	// If this is smaller than MAX_SHORT_BYTES the string is saved in m_Data.raw otherwise in m_Data.ptr;
	int m_Allocated;

	/*
		Contains the number of bytes in the string.
		Without NUL
	*/
	int m_Size;
};

class NulterminatedStringViewWrapper
{
public:
	NulterminatedStringViewWrapper(StringView view) :
		m_Str(view)
	{
	}
	operator const char*() const { return m_Str.Data(); }
private:
	String m_Str;
};

//! Concat two strings.
inline String operator+(const String& a, const String& b)
{
	String out;
	out.Reserve(a.Size() + b.Size());
	out.Append(a);
	out.Append(b);
	return out;
}
inline String operator+(const char* str, const String& string)
{
	return String(str) + string;
}
inline String operator+(const String& string, const char* str)
{
	return string + String(str);
}

inline bool operator==(const char* other, const String& string)
{
	return string.Equal(StringView(other, strlen(other)));
}
inline bool operator==(const String& string, const char* other)
{
	return string.Equal(StringView(other, strlen(other)));
}
inline bool operator==(const String& a, const String& b)
{
	return a.Equal(b);
}
inline bool operator==(const String& a, const StringView& b)
{
	return a.Equal(b);
}
inline bool operator==(const StringView& a, const String& b)
{
	return b.Equal(a);
}

inline bool operator!=(const char* other, const String& string)
{
	return string.Equal(StringView(other, strlen(other)));
}
inline bool operator!=(const String& string, const char* other)
{
	return !string.Equal(StringView(other, strlen(other)));
}
inline bool operator!=(const String& a, const String& b)
{
	return !a.Equal(b);
}
inline bool operator!=(const StringView& a, const String& b)
{
	return !b.Equal(a);
}
inline bool operator!=(const String& a, const StringView& b)
{
	return !a.Equal(b);
}

//! Smaller comparision for strings.
/**
This establishs a total order over all strings.
This order doesn't have to be equal to the lexical order.
*/
inline bool operator<(const String& a, const char* b)
{
	return a.Smaller(StringView(b, strlen(b)));
}
inline bool operator<(const char* a, const String& b)
{
	return StringView(a, strlen(a)).Smaller(b);
}
inline bool operator<(const String& a, const String& b)
{
	return a.Smaller(b);
}

class SharedString
{
private:
	struct Data
	{
		int refCount;
		String str;
	};
	Data* ptr;
public:
	SharedString() :
		ptr(nullptr)
	{
	}
	explicit SharedString(const String& str)
	{
		ptr = LUX_NEW(Data);
		ptr->str = str;
		ptr->refCount = 1;
	}
	explicit SharedString(const char* str) :
		SharedString(String(str))
	{
	}
	SharedString(const SharedString& other)
	{
		ptr = other.ptr;
		if(ptr)
			ptr->refCount++;
	}
	~SharedString()
	{
		if(ptr) {
			ptr->refCount--;
			if(ptr->refCount == 0)
				LUX_FREE(ptr);
		}
	}
	SharedString& operator=(const SharedString& other)
	{
		this->~SharedString();
		ptr = other.ptr;
		if(ptr)
			ptr->refCount++;
		return *this;
	}

	String* operator->() { return &ptr->str; }
	const String* operator->() const { return &ptr->str; }
	String& operator*() { return ptr->str; }
	const String& operator*() const { return ptr->str; }
	operator bool() const { return ptr != nullptr; }

	bool operator==(const SharedString& other) const
	{
		if(ptr == other.ptr)
			return true;
		if(!ptr || !other.ptr)
			return false;
		return ptr->str == other.ptr->str;
	}

	bool operator!=(const SharedString& other) const
	{
		return !(*this == other);
	}

	bool operator<(const SharedString& other) const
	{
		if(ptr == other.ptr)
			return false;
		if(!ptr)
			return true;
		if(!other.ptr)
			return false;
		return ptr->str < other.ptr->str;
	}

	bool operator==(const char* other) const
	{
		return ptr->str == other;
	}

	bool operator!=(const char* other) const
	{
		return !(*this == other);
	}

	bool operator<(const char* other) const
	{
		return ptr->str < other;
	}

	bool operator==(const String& other) const
	{
		return ptr->str == other;
	}

	bool operator!=(const String& other) const
	{
		return !(*this == other);
	}

	bool operator<(const String& other) const
	{
		return ptr->str < other;
	}
};

namespace Types
{
LUX_API Type String();
} // namespace Types

template<> struct TemplType<String> { static Type Get() { return Types::String(); } };

template <>
struct HashType<StringView>
{
	int operator()(StringView view) const
	{
		return (*this)(view.Data(), view.Size());
	}
	int operator()(const char* str, int size) const
	{
		if(size == 0)
			return 0;
		return HashSequence(reinterpret_cast<const u8*>(str), size);
	}
};

template <>
struct HashType<String>
{
	int operator()(const String& str) const
	{
		return HashType<StringView>()(str.Data(), str.Size());
	}
};

template <>
struct HashType<SharedString>
{
	int operator()(const SharedString& t) const
	{
		core::HashType<core::String> hasher;
		if(t)
			return hasher(*t);
		else
			return 0;
	}
	int operator()(const core::String& t) const
	{
		core::HashType<core::String> hasher;
		return hasher(t);
	}
	int operator()(const char* str) const
	{
		core::HashType<core::String> hasher;
		return hasher(str);
	}
};

template <>
struct CompareType<SharedString>
{
	template <typename T>
	bool Equal(const SharedString& str, const T& b) const
	{
		return str == b;
	}
	template <typename T>
	bool Smaller(const SharedString& str, const T& b) const
	{
		return str < b;
	}
};

} // namespace core
} // namespace lux

#endif