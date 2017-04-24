#ifndef INCLUDED_LXSTRING_H
#define INCLUDED_LXSTRING_H
#include "lxMemory.h"
#include "lxCharacter.h"
#include "lxUnicode.h"
#include "lxUtil.h"
#include "lxIterator.h"

namespace lux
{

//! The type of characters a string contains.
DEFINE_FLAG_ENUM_CLASS(EStringType)
{
	Digit = 1,      //!< The string only contains digits.
		Alpha = 2,      //!< The string only contains letters.
		AlphaNum = 4,   //!< The string only contains digits and letters.
		Space = 8,      //!< The string only contains whitespace.
		Upper = 16,     //!< All the characters in the string are upper-case.
		Lower = 32,     //!< All the characters in the string are lower-case.
		Empty = 64,     //!< The string was empty
};

class string;

//! Dummy string type
/**
Alias type for full strings or character-arrays.
Contains UTF8-Data.
Can be used to speed up a function receiving character pointers and strings.
*/
struct string_type
{
	//! Create a dummy string from c-string
	/**
	\param str A nul-terminated c-string, must not be null.
	*/
	string_type(const char* str);
	//! Create a dummy string form a lux::string.
	string_type(const string& str);

	//! Create a dummy string from a pointer
	/**
	\param str A nul-terminated c-string, must not be null.
	\param s The number of bytes in the string, wihtout the NUL-Byte.
	*/
	string_type(const char* str, size_t s) :
		size(s),
		data(str)
	{}

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
class LUX_API string
{
public:
	// Iterator over the codepoints of the string.
	class ConstIterator : public core::BaseIterator<core::BidirectionalIteratorTag, u32>
	{
		friend class string;
	public:
		//! Creates a invalid iterator.
		ConstIterator() :
			m_Data(nullptr),
			m_First(nullptr)
		{
		}

	public:
		//! The invalid iterator.
		/**
		This iterator is always invalid.
		*/
		LUX_API static const ConstIterator INVALID;

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
	static const string EMPTY;

public:
	//! Creates a empty string.
	string();
	//! Create a string from a c-string.
	/**
	\param data A pointer to nul-terminated string data, if null a empty string is created.
	\param length The number of character to copy from the string if SIZE_T_MAX all characters a copied.
	*/
	string(const char* data, size_t length = std::numeric_limits<size_t>::max());

	//! Copyconstructor
	string(const string& other);
	//! Moveconstructor
	string(string&& old);

	//! Destructor
	~string();

	//! Creates a copy of this string.
	string Copy();

	//! Reserve size bytes for string memory.
	/**
	After this call Data() will point to size+1 bytes of memory.
	\param size The number of bytes to allocate for string-storage without terminating NUL.
	*/
	void Reserve(size_t size);

	//! Copy-assignment
	string& operator=(const string& other);
	//! Copy-assignment
	string& operator=(const char* other);
	//! Move-assignment
	string& operator=(string&& old);

	//! Append another string
	string& operator+=(const string_type& str);
	//! Concat two strings.
	string operator+(const string_type& str) const;

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	bool operator==(const string_type& other) const;
	//! Compare two strings for inequality
	/*
	No unicode normalization is performed
	*/
	bool operator!=(const string_type& other) const;

	//! Smaller comparision for strings.
	/**
	This establishs a total order over all strings.
	This order doesn't have to be equal to the lexical order.
	*/
	bool operator<(const string_type& other) const;

	//! Compare two strings for equality
	/*
	No unicode normalization is performed
	*/
	bool Equal(const string_type& other) const;

	//! Compare two strings for equality, case insensitive.
	/*
	No unicode normalization is performed
	*/
	bool EqualCaseInsensitive(const string_type& other) const;

	//! Insert another string into this one.
	/**
	\param pos The location of the first inserted character.
	\param other The string to insert.
	\param count The number of characters to insert, SIZE_T_MAX to insert all characters.
	\return A iterator to the first character after the inserted part of the string.
	*/
	ConstIterator Insert(ConstIterator pos, const string_type& other, size_t count = std::numeric_limits<size_t>::max());

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
	string& AppendRaw(const char* data, size_t bytes);

	//! Append another string onto this one.
	/**
	\param other The string to append.
	\param count The number of characters to append, SIZE_T_MAX to append all characters.
	\return selfreference
	*/
	string& Append(const string_type& other, size_t count = std::numeric_limits<size_t>::max());

	//! Append another string onto this one.
	/**
	\param first The first iterator of the range to append.
	\param end The end iterator of the range to append.
	\return selfreference
	*/
	string& Append(ConstIterator first, ConstIterator end);

	//! Append a single character from a iterator.
	/**
	\param character The character referenced by this iterator is appended.
	\return selfreference
	*/
	string& Append(ConstIterator character);

	//! Append a single character.
	/**
	\param character A unicode-codepoint to append
	\return selfreference
	*/
	string& Append(u32 character);

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
	void Resize(size_t newLength, const string_type& filler = " ");

	//! Clear the string contents, making the string empty.
	void Clear();

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

	//! Test if the string starts with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the First() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool StartsWith(const string_type& data, ConstIterator first = ConstIterator::INVALID) const;

	//! Test if the string ends with a given string.
	/**
	\param data The string to test with.
	\param first The position from where the test is performed, if invalid the End() iterator is used.
	\param True, if this string starts with the given one, false otherwise
	*/
	bool EndsWith(const string_type& data, ConstIterator end = ConstIterator::INVALID) const;

	//! Replace all occurences of a substring in this string.
	/**
	This method doesn't work recursive.
	\param replace The string which the search string is replaced with.
	\param search The string to search for, my not be empty.
	\param first The iterator from where to start the search, if invalid First() is used.
	\param end The iterator where the search is stopped, if invalid End() is used.
	\return The number of occurences found and replaced.
	*/
	size_t Replace(const string_type& replace, const string_type& search, ConstIterator first = ConstIterator::INVALID, ConstIterator end = ConstIterator::INVALID);

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with
	\param rangeFirst The first iterator of the replaced range
	\param rangeEnd the end of the replace range
	\return A iterator to the first character after the newly inserted string.
	*/
	ConstIterator ReplaceRange(const string_type& replace, ConstIterator rangeFirst, ConstIterator rangeEnd = ConstIterator::INVALID);

	//! Replace a range of a string with a given string.
	/**
	\param replace The string to replace the range with.
	\param rangeFirst The first iterator of the replaced range.
	\param count The number of characters to replace.
	\return A iterator to the first character after the newly inserted string.
	*/
	ConstIterator ReplaceRange(const string_type& replace, ConstIterator rangeFirst, size_t count);

	//! Find the first occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	ConstIterator Find(const string_type& search, ConstIterator first = ConstIterator::INVALID, ConstIterator end = ConstIterator::INVALID) const;

	//! Find the last occurence of a substring in this string.
	/**
	\param search The string to search for, the empty string is never found.
	\param first The position where the search should begin, if invalid First() is used.
	\param end The position where the search should end, if invalid End() is used.
	\return A iterator to the first character of the searched string, or the used end if it couldn't be found.
	*/
	ConstIterator FindReverse(const string_type& search, ConstIterator first = ConstIterator::INVALID, ConstIterator end = ConstIterator::INVALID) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param count The number of character to extract.
	\return The extracted substring
	*/
	string SubString(ConstIterator first, size_t count = 1) const;

	//! Extract a substring from this string.
	/**
	\param first The first character of the string to extract.
	\param end The character after the last one to extract.
	\return The extracted substring
	*/
	string SubString(ConstIterator first, ConstIterator end) const;

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
	string& RStrip(ConstIterator end = ConstIterator::INVALID);

	//! Removes all whitespace from the left side of the string.
	/**
	Whitespace characters are: \t\n\r and space.
	Character are removed from the front of the string, until a not space character is encountered.
	\param first Where should the removing of characters start, if invalid First() is used.
	\return selfreference
	*/
	string& LStrip(ConstIterator first = ConstIterator::INVALID);

	//! Classify the content of the string
	/**
	See \ref{EStringType} for more information about string classification.
	*/
	EStringType Classify() const;

private:
	//! Is the string saved in short format.
	/**
	i.e. As bytes in the value of m_Data
	*/
	bool IsShortString() const;
	//! Returns the number of allocated bytes, including NUL.
	size_t GetAllocated() const;
	//! Set the number of allocated bytes, including NUL.
	/**
	\param a The number of allocated bytes, including NUL.
	\param short_string Is the string a short string.
	*/
	void SetAllocated(size_t a, bool short_string = false);

	//! The maximum number of bytes contained in a short-string.
	size_t MaxShortStringBytes() const;

	//! Push a single character to string, it's data is read from ptr.
	void PushCharacter(const char* ptr);
	
private:
	//! Contains the raw data of the string.
	/**
	Never access this member always used the Data() function.
	The string is saved in utf-8 format and is null-terminated.
	Can be null to represent the empty string.
	Can contain the content of a short string direclty.
	*/
	char* m_Data;

	/*
		Contains the number of bytes in the string.
		Without NUL
	*/
	size_t m_Size;

	// The size of the array pointed to by m_Data, if m_Data is a pointer.
	// If the high-order bit is set, this is a short string, m_Data contains the string-data itself.
	// With NUL
	size_t m_Allocated;

	/*
	Contains the number of codepoints in the string.
	*/
	size_t m_Length;
};

inline string operator+(const char* str, const string& string)
{
	return string + str;
}

inline bool operator==(const char* other, const string& string)
{
	return string == other;
}

inline bool operator!=(const char* other, const string& string)
{
	return string != other;
}


inline string_type::string_type(const char* str) :
	size(0xFFFFFFFF),
	data(str)
{
}

inline string_type::string_type(const string& str) :
	size(str.Size()),
	data(str.Data())
{
}

inline void string_type::EnsureSize() const
{
	if(size == 0xFFFFFFFF)
		size = strlen(data);
}

namespace io
{
using path = string;
}

namespace core
{
template <>
struct HashType<string>
{
	size_t operator()(const string& str) const
	{
		if(str.IsEmpty())
			return 0;

		return HashSequence(reinterpret_cast<const u8*>(str.Data()), str.Size());
	}
};
}

}    


#endif