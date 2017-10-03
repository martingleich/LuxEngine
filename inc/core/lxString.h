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

}
//! The type of characters a string contains.
enum class EStringType
{
	Digit = 1,      //!< The string only contains digits.
	Alpha = 2,      //!< The string only contains letters.
	AlphaNum = 4,   //!< The string only contains digits and letters.
	Space = 8,      //!< The string only contains whitespace.
	Upper = 16,     //!< All the characters in the string are upper-case.
	Lower = 32,     //!< All the characters in the string are lower-case.
	Empty = 64,     //!< The string was empty
};

DECLARE_FLAG_CLASS(EStringType);

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
	StringType(const char* str);
	//! Create a dummy string form a lux::string.
	StringType(const String& str);

	//! Create a dummy string from a pointer
	/**
	\param str A nul-terminated c-string, must not be null.
	\param s The number of bytes in the string, wihtout the NUL-Byte.
	*/
	StringType(const char* str, size_t s) :
		size(s),
		data(str)
	{
	}

	//! Ensures that the size of string is available
	/**
	Should be called at least one before accessing the size property.
	*/
	void EnsureSize() const;

	//! The number of bytes in the string, without the NUL-Byte.
	mutable size_t size;

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
class LUX_API String
{
public:
	//! Iterator over the codepoints of the string.
	class ConstIterator : public core::BaseIterator<core::BidirectionalIteratorTag, u32>
	{
		friend class String;
	public:
		//! Creates a invalid iterator.
		ConstIterator() :
			m_Data(nullptr),
			m_First(nullptr)
		{
		}

		static ConstIterator Invalid()
		{
			return ConstIterator();
		}

		ConstIterator& operator++()
		{
			if(m_Data < m_First)
				++m_Data;
			else
				core::AdvanceCursorUTF8(m_Data);
			return *this;
		}
		ConstIterator& operator--()
		{
			if(m_Data <= m_First)
				--m_Data;
			else
				core::RetractCursorUTF8(m_Data);
			return *this;
		}
		ConstIterator operator++(int)
		{
			ConstIterator tmp(*this);
			++*this;
			return tmp;
		}

		ConstIterator operator--(int)
		{
			ConstIterator tmp(*this);
			++*this;
			return tmp;
		}

		ConstIterator Next()
		{
			ConstIterator tmp(*this);
			++tmp;
			return tmp;
		}

		ConstIterator Prev()
		{
			ConstIterator tmp(*this);
			--tmp;
			return tmp;
		}

		ConstIterator& operator+=(intptr_t i)
		{
			if(i < 0)
				*this -= -i;
			while(i--)
				++*this;
			return *this;
		}

		ConstIterator& operator-=(intptr_t i)
		{
			if(i < 0)
				*this += -i;
			while(i--)
				--*this;
			return *this;
		}

		ConstIterator operator+(intptr_t i)
		{
			ConstIterator out(*this);
			out += i;
			return out;
		}

		ConstIterator operator-(intptr_t i)
		{
			ConstIterator out(*this);
			out -= i;
			return out;
		}

		bool operator==(ConstIterator other) const
		{
			return m_Data == other.m_Data;
		}
		bool operator!=(ConstIterator other) const
		{
			return !(*this == other);
		}
		bool operator<(ConstIterator other) const
		{
			return m_Data < other.m_Data;
		}

		bool operator<=(ConstIterator other) const
		{
			return m_Data <= other.m_Data;
		}

		bool operator>(ConstIterator other) const
		{
			return m_Data > other.m_Data;
		}

		bool operator>=(ConstIterator other) const
		{
			return m_Data >= other.m_Data;
		}

		u32 operator*() const
		{
			return core::GetCharacterUTF8(m_Data);
		}

		//! Access the character the iterator is referencing
		/**
		All valid string iterators point to a continous block of memory, inside
		a null-terminated string.
		\return Pointer to the character referenced by the iterator.
		*/
		const char* Pointer() const
		{
			return m_Data;
		}

	private:
		//! Create a iterator.
		/**
		\param ptr The character reference by the iterator.
		\param first The first character of the string referenced.
		*/
		explicit ConstIterator(const char* ptr, const char* first) :
			m_Data(ptr),
			m_First(first)
		{
		}

	private:
		const char* m_Data; //!< The character referenced by the iterator, always the first element of a utf8 sequence if valid.
		const char* m_First; //!< The first character of the string containing the iterator.
	};

public:
	//! The empty string.
	/**
	Can be used to pass referenced without creating a temporary.
	*/
	static const String EMPTY;

public:
	//! Creates a empty string.
	String();
	//! Create a string from a c-string.
	/**
	\param data A pointer to nul-terminated string data, if null a empty string is created.
	\param length The number of character to copy from the string if SIZE_T_MAX all characters a copied.
	*/
	String(const char* data, size_t length = std::numeric_limits<size_t>::max());
	String(ConstIterator first, ConstIterator end);

	//! Copyconstructor
	String(const String& other);
	//! Moveconstructor
	String(String&& old);

	//! Destructor
	~String();

	//! Creates a copy of this string.
	String Copy();

	//! Reserve size bytes for string memory.
	/**
	After this call Data() will point to size+1 bytes of memory.
	\param size The number of bytes to allocate for string-storage without terminating NUL.
	*/
	void Reserve(size_t size);

	//! Copy-assignment
	String& operator=(const String& other);
	//! Copy-assignment
	String& operator=(const char* other);
	//! Move-assignment
	String& operator=(String&& old);

	//! Append another string
	String& operator+=(const StringType& str);
	//! Concat two strings.
	String operator+(const StringType& str) const;

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	bool operator==(const StringType& other) const;
	//! Compare two strings for inequality
	/*
	No unicode normalization is performed
	*/
	bool operator!=(const StringType& other) const;

	//! Smaller comparision for strings.
	/**
	This establishs a total order over all strings.
	This order doesn't have to be equal to the lexical order.
	*/
	bool operator<(const StringType& other) const;

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	bool Equal(const StringType& other) const;

	//! Compare two strings for equality, case insensitive.
	/*
	No unicode normalization is performed
	*/
	bool EqualCaseInsensitive(const StringType& other) const;

	//! Compare two strings, case insensitive.
	/*
	No unicode normalization is performed
	*/
	bool SmallerCaseInsensitive(const StringType& other) const;

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted character.
	\param other The string to insert.
	\param count The number of characters to insert, SIZE_T_MAX to insert all characters.
	\return A iterator to the first character after the inserted part of the string.
	*/
	ConstIterator Insert(ConstIterator pos, const StringType& other, size_t count = std::numeric_limits<size_t>::max());

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted character.
	\param first A iterator to the first character to insert.
	\param end The end iterator of the range to insert.
	\return A iterator to the first character after the inserted part of the string.
	*/
	ConstIterator Insert(ConstIterator pos, ConstIterator first, ConstIterator end);

	//! Append a raw block of bytes
	/**
	This method is used to create string from raw-data, read from files or other sources.
	\param data Pointer to the raw data to append to the string, may not be null-terminated.
	\param bytes The number of exact bytes to copy from the string
	\return selfreference
	*/
	String& AppendRaw(const char* data, size_t bytes);

	//! Append another string onto this one.
	/**
	\param other The string to append.
	\param count The number of characters to append, SIZE_T_MAX to append all characters.
	\return selfreference
	*/
	String& Append(const StringType& other, size_t count = std::numeric_limits<size_t>::max());

	//! Append another string onto this one.
	/**
	\param first The first iterator of the range to append.
	\param end The end iterator of the range to append.
	\return selfreference
	*/
	String& Append(ConstIterator first, ConstIterator end);

	//! Append a single character from a iterator.
	/**
	\param character The character referenced by this iterator is appended.
	\return selfreference
	*/
	String& Append(ConstIterator character);

	//! Append a single character.
	/**
	\param character A unicode-codepoint to append
	\return selfreference
	*/
	String& Append(u32 character);

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
	void Resize(size_t newLength, const StringType& filler = " ");

	//! Clear the string contents, making the string empty.
	String& Clear();

	//! The number of bytes contained in the string, without NUL.
	size_t Size() const;

	//! The number of codepoints contained in the string, without NUL.
	size_t Length() const;

	//! Is the string empty.
	/*
	The string is empty if is size and length are zero.
	*/
	bool IsEmpty() const;

	//! Gives access to the raw string-data.
	/**
	A pointer to at least Size()+1 bytes of string data.
	*/
	const char* Data_c() const;
	const char* Data() const;

	//! Gives access to the raw string-data.
	/**
	A pointer to at least Size()+1 bytes of string data.
	Can be used to manipulate the string, use with care.
	*/
	char* Data();

	//! Add a single utf-8 byte to the string.
	/**
	Always call this method so often until a full codepoint was added.
	*/
	void PushByte(u8 byte);

	//! Iterator the the character before the first in the string
	/**
	Can't be dereferenced.
	*/
	ConstIterator Begin() const;

	//! Iterator the the first character in the string.
	ConstIterator First() const;

	//! Iterator the the last character in the string.
	ConstIterator Last() const;

	//! Iterator the the character after the last in the string
	/**
	Can't be dereferenced.
	*/
	ConstIterator End() const;

	//! Support for foreach loop
	ConstIterator begin() const
	{
		return First();
	}

	//! Support for foreach loop
	ConstIterator end() const
	{
		return End();
	}

	//! Test if the string starts with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the First() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool StartsWith(const StringType& data, ConstIterator first = ConstIterator::Invalid()) const;

	//! Test if the string ends with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the End() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool EndsWith(const StringType& data, ConstIterator end = ConstIterator::Invalid()) const;

	//! Replace all occurences of a substring in this string.
	/**
	This method doesn't work recursive.
	\param replace The string which the search string is replaced with.
	\param search The string to search for, my not be empty.
	\param first The iterator from where to start the search, if invalid First() is used.
	\param end The iterator where the search is stopped, if invalid End() is used.
	\return The number of occurences found and replaced.
	*/
	size_t Replace(const StringType& replace, const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid());

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with
	\param rangeFirst The first iterator of the replaced range
	\param rangeEnd the end of the replace range
	\return A iterator to the first character after the newly inserted string.
	*/
	ConstIterator ReplaceRange(const StringType& replace, ConstIterator rangeFirst, ConstIterator rangeEnd = ConstIterator::Invalid());

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with.
	\param rangeFirst The first iterator of the replaced range.
	\param count The number of characters to replace.
	\return A iterator to the first character after the newly inserted string.
	*/
	ConstIterator ReplaceRange(const StringType& replace, ConstIterator rangeFirst, size_t count);

	//! Find the first occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	ConstIterator Find(const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid()) const;

	//! Find the last occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	ConstIterator FindReverse(const StringType& search, ConstIterator first = ConstIterator::Invalid(), ConstIterator end = ConstIterator::Invalid()) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param count The number of character to extract.
	\return The extracted substring
	*/
	String SubString(ConstIterator first, size_t count = 1) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param end The character after the last one to extract.
	\return The extracted substring
	*/
	String SubString(ConstIterator first, ConstIterator end) const;

	//! Removes a number of characters from the back of string.
	/**
	\param count The number of characters to remove, may be bigger than the number of characters in the string.
	\return The actual number of characters removed, if not equal to count, the string will be empty.
	*/
	size_t Pop(size_t count = 1);

	//! Removes characters from the string.
	/**
	\param pos Where should the character be removed.
	\param count The number of characters to remove.
	\return A iterator to the first character after the deleted range.
	*/
	ConstIterator Remove(ConstIterator pos, size_t count = 1);

	//! Removes characters from the string.
	/**
	\param from Where to start removing characters.
	\param end Where to stop removing characters.
	\return A iterator to the first character after the deleted range.
	*/
	ConstIterator Remove(ConstIterator from, ConstIterator to);

	//! Removes all whitespace from the right side of the string.
	/**
	Whitespace characters are: \t\n\r and space.
	Character are removed from the back of the string, until a not space character is encountered.
	\param end Where should the removing of characters start, if invalid End() is used.
	\return selfreference
	*/
	String& RStrip(ConstIterator end = ConstIterator::Invalid());

	//! Removes all whitespace from the left side of the string.
	/**
	Whitespace characters are: \t\n\r and space.
	Character are removed from the front of the string, until a not space character is encountered.
	\param first Where should the removing of characters start, if invalid First() is used.
	\return selfreference
	*/
	String& LStrip(ConstIterator first = ConstIterator::Invalid());

	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\param outArray Where to write the substrings.
	\param maxCount The amount of strings available in the output array.
	\return The number of written output strings.
	*/
	size_t Split(u32 ch, String* outArray, size_t maxCount) const;

	//! Split the string on a character.
	/**
	If the character isn't contained in the string, the original string is returned.
	\param ch The character were to split the string.
	\return The array of substrings
	*/
	core::Array<String> Split(u32 ch) const;

	//! Classify the content of the string
	/**
	See \ref{EStringType} for more information about string classification.
	*/
	EStringType Classify() const;

	//! Get a string in lower case
	String GetLower() const;

	//! Get a string in upper case
	String GetUpper() const;

private:
	//! Is the string saved in short format.
	bool IsShortString() const;

	//! Returns the number of allocated bytes, including NUL.
	size_t GetAllocated() const;

	//! Set the number of allocated bytes, including NUL.
	void SetAllocated(size_t a);

	//! Push a single character to string, it's data is read from ptr.
	void PushCharacter(const char* ptr);

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

	static const size_t MAX_SHORT_BYTES = sizeof(DataUnion);

	// The number of bytes allocated for the string, including the terminating NULL character.
	// If this is smaller than MAX_SHORT_BYTES the string is saved in m_Data.raw otherwise in m_Data.ptr;
	size_t m_Allocated;

	/*
		Contains the number of bytes in the string.
		Without NUL
	*/
	size_t m_Size;

	/*
	Contains the number of codepoints in the string.
	*/
	size_t m_Length;
};

inline String operator+(const char* str, const String& string)
{
	return lux::String(str) + string;
}

inline bool operator==(const char* other, const String& string)
{
	return string == other;
}

inline bool operator!=(const char* other, const String& string)
{
	return string != other;
}


inline StringType::StringType(const char* str) :
	size(0xFFFFFFFF),
	data(str)
{
}

inline StringType::StringType(const String& str) :
	size(str.Size()),
	data(str.Data())
{
}

inline void StringType::EnsureSize() const
{
	if(size == 0xFFFFFFFF)
		size = strlen(data);
}

namespace io
{
using Path = String;
}

namespace core
{
namespace Types
{
LUX_API Type String();
} // namespace Types

template<> inline Type GetTypeInfo<String>() { return Types::String(); }

template <>
struct HashType<String>
{
	size_t operator()(const String& str) const
	{
		if(str.IsEmpty())
			return 0;

		return HashSequence(reinterpret_cast<const u8*>(str.Data()), str.Size());
	}
};

} // namespace core
} // namespace lux

#endif