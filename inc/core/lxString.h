#ifndef INCLUDED_LXSTRING_H
#define INCLUDED_LXSTRING_H
#include "lxMemory.h"
#include "lxUnicode.h"
#include "lxUtil.h"
#include "lxIterator.h"
#include "lxException.h"
#include "lxTypes.h"

#include <limits>

namespace lux
{
namespace core
{
template <typename T>
class Array;

//! The type of characters a string contains.
enum class EStringClass
{
	Digit = 1,      //!< The string only contains digits.
	Alpha = 2,      //!< The string only contains letters.
	AlphaNum = 4,   //!< The string only contains digits and letters.
	Space = 8,      //!< The string only contains whitespace.
	Upper = 16,     //!< All the characters in the string are upper-case.
	Lower = 32,     //!< All the characters in the string are lower-case.
	Empty = 64,     //!< The string was empty
};

class String;

//! Dummy string type
/**
Alias type for full strings or character-arrays.
Contains UTF8-Data.
Can be used to speed up a function receiving character pointers and strings.
*/
struct StringType
{
	//! Create a dummy string from c-string
	/**
	\param str A nul-terminated c-string, must not be null.
	*/
	StringType(const char* str) :
		size(-1),
		data(str)
	{
	}


	//! Create a dummy string from a pointer
	/**
	\param str A nul-terminated c-string, must not be null.
	\param s The number of bytes in the string, wihtout the NUL-Byte.
	*/
	StringType(const char* str, int s) :
		size(s),
		data(str)
	{
	}

	//! Ensures that the size of string is available
	/**
	Should be called at least one before accessing the size property.
	*/
	void EnsureSize() const
	{
		if(size < 0)
			size = (int)strlen(data);
	}

	//! The number of bytes in the string, without the NUL-Byte.
	mutable int size;

	//! Pointer to the string-data
	/**
	Nul-Terminated c-string.
	*/
	const char* data;
};

//! A utf8-string
/**
All non-constant methods invalidate all iterators.
No string can contain the NUL character.
*/
class String
{
public:
	using ConstIterator = ConstUTF8Iterator;

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
	\param length The number of character to copy from the string if -1 all characters a copied.
	*/
	LUX_API String(const char* data, int length = -1);
	LUX_API String(ConstIterator first, ConstIterator end);

	//! Copyconstructor
	LUX_API String(const String& other);
	//! Moveconstructor
	LUX_API String(String&& old);

	//! Destructor
	LUX_API ~String();

	//! Creates a copy of this string.
	LUX_API String Copy();

	operator StringType() const
	{
		return StringType(Data_c(), Size());
	}

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
	LUX_API String& operator+=(const StringType& str);

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	LUX_API bool Equal(const StringType& other) const;

	//! Compare two strings for equality, case insensitive.
	/*
	No unicode normalization is performed
	*/
	LUX_API bool EqualCaseInsensitive(const StringType& other) const;

	//! Compare two strings, case insensitive.
	/*
	No unicode normalization is performed
	*/
	LUX_API bool SmallerCaseInsensitive(const StringType& other) const;

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted character.
	\param other The string to insert.
	\param count The number of characters to insert, -1 to insert all characters.
	\return A iterator to the first character after the inserted part of the string.
	*/
	LUX_API ConstIterator Insert(ConstIterator pos, const StringType& other, int count = -1);

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted character.
	\param first A iterator to the first character to insert.
	\param end The end iterator of the range to insert.
	\return A iterator to the first character after the inserted part of the string.
	*/
	LUX_API ConstIterator Insert(ConstIterator pos, ConstIterator first, ConstIterator end);

	//! Append a raw block of bytes
	/**
	This method is used to create string from raw-data, read from files or other sources.
	\param data Pointer to the raw data to append to the string, may not be null-terminated.
	\param bytes The number of exact bytes to copy from the string
	\return selfreference
	*/
	LUX_API String& AppendRaw(const char* data, int bytes);

	//! Append another string onto this one.
	/**
	\param other The string to append.
	\param count The number of characters to append, -1 to append all characters.
	\return selfreference
	*/
	LUX_API String& Append(const StringType& other, int count = -1);

	//! Append another string onto this one.
	/**
	\param first The first iterator of the range to append.
	\param end The end iterator of the range to append.
	\return selfreference
	*/
	LUX_API String& Append(ConstIterator first, ConstIterator end);

	//! Append a single character from a iterator.
	/**
	\param character The character referenced by this iterator is appended.
	\return selfreference
	*/
	LUX_API String& Append(ConstIterator character);

	//! Append a single character.
	/**
	\param character A unicode-codepoint to append
	\return selfreference
	*/
	LUX_API String& Append(u32 character);

	//! Resize this string
	/**
	If the new length is smaller of equal to the current length, the end of the string
	is cut away so it's new Length is achived.
	If the new length is biffer than the current length, character are read from filler
	and added to the string until the new length is reached, if more character are needed than filler contains,
	character are read from the start again.
	\param newLength The new lenght of the string.
	\param filler The character to fill the newly created string with
	*/
	LUX_API void Resize(int newLength, const StringType& filler = " ");

	//! Clear the string contents, making the string empty.
	LUX_API String& Clear();

	//! The number of bytes contained in the string, without NUL.
	inline int Size() const
	{
		return m_Size;
	}

	//! The number of bytes allocated for the string.
	inline int Allocated() const
	{
		return m_Allocated - 1;
	}

	//! The number of codepoints contained in the string, without NUL.
	inline int Length() const
	{
		return m_Length;
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
	LUX_API const char* Data_c() const;
	LUX_API const char* Data() const;

	//! Gives access to the raw string-data.
	/**
	A pointer to at least Size()+1 bytes of string data.
	Can be used to manipulate the string, use with care.
	*/
	LUX_API char* Data();

	//! Add a single utf-8 byte to the string.
	/**
	Always call this method so often until a full codepoint was added.
	*/
	LUX_API void PushByte(u8 byte);

	//! Iterator the the character before the first in the string
	/**
	Can't be dereferenced.
	*/
	inline ConstIterator Begin() const
	{
		return ConstIterator(Data_c() - 1, Data_c());
	}

	//! Iterator the the first character in the string.
	inline ConstIterator First() const
	{
		return ConstIterator(Data_c(), Data_c());
	}

	//! Iterator the the last character in the string.
	inline ConstIterator Last() const
	{
		if(m_Size > 0)
			return End() - 1;
		else
			return End();
	}

	//! Iterator the the character after the last in the string
	/**
	Can't be dereferenced.
	*/
	inline ConstIterator End() const
	{
		return ConstIterator(Data_c() + m_Size, Data_c());
	}

	//! Test if the string starts with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the First() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	LUX_API bool StartsWith(const StringType& data, ConstIterator first = ConstIterator::Invalid()) const;

	//! Test if the string ends with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the End() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	LUX_API bool EndsWith(const StringType& data, ConstIterator end = ConstIterator::Invalid()) const;

	//! Replace all occurences of a substring in this string.
	/**
	This method doesn't work recursive.
	\param replace The string which the search string is replaced with.
	\param search The string to search for, my not be empty.
	\param first The iterator from where to start the search, if invalid First() is used.
	\param end The iterator where the search is stopped, if invalid End() is used.
	\return The number of occurences found and replaced.
	*/
	LUX_API int Replace(const StringType& replace, const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid());

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with
	\param rangeFirst The first iterator of the replaced range
	\param rangeEnd the end of the replace range
	\return A iterator to the first character after the newly inserted string.
	*/
	LUX_API ConstIterator ReplaceRange(const StringType& replace, ConstIterator rangeFirst, ConstIterator rangeEnd = ConstIterator::Invalid());

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with.
	\param rangeFirst The first iterator of the replaced range.
	\param count The number of characters to replace.
	\return A iterator to the first character after the newly inserted string.
	*/
	LUX_API ConstIterator ReplaceRange(const StringType& replace, ConstIterator rangeFirst, int count);

	//! Find the first occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	LUX_API ConstIterator Find(const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid()) const;

	//! Find the last occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	LUX_API ConstIterator FindReverse(const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid()) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param count The number of character to extract.
	\return The extracted substring
	*/
	LUX_API String SubString(ConstIterator first, int count = 1) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param end The character after the last one to extract.
	\return The extracted substring
	*/
	LUX_API String SubString(ConstIterator first, ConstIterator end) const;

	//! Removes a number of characters from the back of string.
	/**
	\param count The number of characters to remove, may be bigger than the number of characters in the string.
	\return The actual number of characters removed, if not equal to count, the string will be empty.
	*/
	LUX_API int Pop(int count = 1);

	//! Removes characters from the string.
	/**
	\param pos Where should the character be removed.
	\param count The number of characters to remove.
	\return A iterator to the first character after the deleted range.
	*/
	LUX_API ConstIterator Remove(ConstIterator pos, int count = 1);

	//! Removes characters from the string.
	/**
	\param from Where to start removing characters.
	\param end Where to stop removing characters.
	\return A iterator to the first character after the deleted range.
	*/
	LUX_API ConstIterator Remove(ConstIterator from, ConstIterator to);

	//! Removes all whitespace from the right side of the string.
	/**
	Whitespace characters are: \t\n\r and space.
	Character are removed from the back of the string, until a not space character is encountered.
	\param end Where should the removing of characters start, if invalid End() is used.
	\return selfreference
	*/
	LUX_API String& RStrip(ConstIterator end = ConstIterator::Invalid());

	//! Removes all whitespace from the left side of the string.
	/**
	Whitespace characters are: \t\n\r and space.
	Character are removed from the front of the string, until a not space character is encountered.
	\param first Where should the removing of characters start, if invalid First() is used.
	\return selfreference
	*/
	LUX_API String& LStrip(ConstIterator first = ConstIterator::Invalid());

	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\param outArray Where to write the substrings.
	\param maxCount The amount of strings available in the output array.
	\param ignoreEmpty Empty split strings aren't added to the output
	\return The number of written output strings.
	*/
	LUX_API int Split(u32 ch, String* outArray, int maxCount, bool ignoreEmpty = false) const;

	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\param ignoreEmpty Empty split strings aren't added to the output
	\return The array of substrings
	*/
	LUX_API core::Array<String> Split(u32 ch, bool ignoreEmpty = false) const;

	//! Classify the content of the string
	/**
	See \ref{EStringType} for more information about string classification.
	*/
	LUX_API EStringClass Classify() const;

	//! Contains the string only whitespace(or is empty)
	LUX_API bool IsWhitespace() const;

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

	//! Push a single character to string, it's data is read from ptr.
	LUX_API void PushCharacter(const char* ptr);

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

	/*
	Contains the number of codepoints in the string.
	*/
	int m_Length;
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
	return string.Equal(other);
}
inline bool operator==(const String& string, const char* other)
{
	return string.Equal(other);
}
inline bool operator==(const String& a, const String& b)
{
	return a.Equal(b);
}
inline bool operator==(const String& a, const StringType& b)
{
	return a.Equal(b);
}
inline bool operator==(const StringType& a, const String& b)
{
	return b.Equal(a);
}
inline bool operator!=(const char* other, const String& string)
{
	return string.Equal(other);
}
inline bool operator!=(const String& string, const char* other)
{
	return !string.Equal(other);
}
inline bool operator!=(const String& a, const String& b)
{
	return !a.Equal(b);
}
inline bool operator!=(const StringType& a, const String& b)
{
	return !b.Equal(a);
}
inline bool operator!=(const String& a, const StringType& b)
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
	auto size = (int)strlen(b);
	auto s = a.Size() < size ? a.Size() : size;
	return (memcmp(a.Data(), b, s) < 0);
}
inline bool operator<(const char* a, const String& b)
{
	auto size = (int)strlen(a);
	auto s = size < b.Size() ? size : b.Size();
	return (memcmp(a, b.Data(), s) < 0);
}
inline bool operator<(const String& a, const String& b)
{
	auto s = a.Size() < b.Size() ? a.Size() : b.Size();
	return (memcmp(a.Data(), b.Data(), s) < 0);
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

inline String::ConstIterator begin(const String& str) { return str.First(); }
inline String::ConstIterator end(const String& str) { return str.End(); }

namespace Types
{
LUX_API Type String();
} // namespace Types

template<> struct TemplType<String> { static Type Get() { return Types::String(); } };

template <>
struct HashType<String>
{
	int operator()(const String& str) const
	{
		return (*this)(str.Data(), str.Size());
	}
	int operator()(const char* str, int size) const
	{
		if(size == 0)
			return 0;
		return HashSequence(reinterpret_cast<const u8*>(str), size);
	}
	int operator()(const char* str) const
	{
		int size = (int)strlen(str);
		return (*this)(str, size);
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
namespace io
{
using Path = core::String;
}

DECLARE_FLAG_CLASS(core::EStringClass);

} // namespace lux

#endif