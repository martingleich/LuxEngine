#ifndef INCLUDED_FORMAT_STRING_BASICS_H
#define INCLUDED_FORMAT_STRING_BASICS_H
#include "format/StringType.h"
#include "format/Exception.h"
#include <inttypes.h>

namespace format
{

//! Stringlength of some fixed-width character string, null-terminated
template <typename T>
inline size_t StringLengthFixed(const char* str)
{
	const T* s = (const T*)str;
	size_t len = 0;
	while(*s++)
		++len;
	return len;
}

//! Stringlenght of an utf8 string, null-terminated
inline size_t StringLengthUnicode(const char* str)
{
	size_t len = 0;
	const uint8_t* s = (const uint8_t*)str;
	while(*s) {
		if((*s & 0xC0) != 0x80)
			++len;
		++s;
	}

	return len;
}

//! Returns the number of codepoints in the string, null-termainted
inline size_t StringLength(StringType type, const char* str)
{
	switch(type) {
	case Unicode:
		return StringLengthUnicode(str);
	case Ascii:
		return StringLengthFixed<uint8_t>(str);
	case CodePoint:
		return StringLengthFixed<uint32_t>(str);
	default:
		throw exception();
	}
}

//! Append string data from dst to src
/**
\param dst Data is written here, the first character of src is put into dst[0] and so on, must have enough room to contain the string
\param src String data is read from here, must be null-terminated
\return Pointer to the first character after the new string
*/
inline char* AppendStr(char* dst, const char* src)
{
	const uint8_t* s = (const uint8_t*)src;
	uint8_t* d = (uint8_t*)dst;
	while(*s)
		*d++ = *s++;

	return (char*)d;
}

//! Read a single character from a fixed-width string
template <typename T>
inline uint32_t GetCharacterFixed(const char* ptr)
{
	return static_cast<T>(*(reinterpret_cast<const T*>(ptr)));
}

//! Read a single character from a utf8 string
inline uint32_t GetCharacterUnicode(const char* ptr)
{
	uint8_t u0 = ptr[0];
	if((u0 & 0x80) == 0) // 0xxxxxxx
		return (uint32_t)u0;
	uint8_t u1 = ptr[1];
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	uint8_t u2 = ptr[2];
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	uint8_t u3 = ptr[3];
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC) << 0;
	uint8_t u4 = ptr[4];
	if((u0 & 0xFD) == 0xF8) // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xFD) << 24 | (u1&~0xC0) << 18 | (u2&~0xC0) << 12 | (u3 &~0xC) << 6 | (u4 &~0xC) << 0;
	uint8_t u5 = ptr[4];
	if((u0 & 0xFE) == 0xFD) // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xFE) << 30 | (u1&~0xC0) << 24 | (u2&~0xC0) << 18 | (u3 &~0xC) << 12 | (u4 &~0xC) << 6 | (u5 &~0xC) << 0;

	return 0;
}

//! Read a single character from any string type
inline uint32_t GetCharacter(StringType type, const char* str)
{
	switch(type) {
	case Unicode:
		return GetCharacterUnicode(str);
	case Ascii:
		return GetCharacterFixed<uint8_t>(str);
	case CodePoint:
		return GetCharacterFixed<uint32_t>(str);
	default:
		throw exception();
	}
}

//! Advance a string cursor in a fixed-width string
/**
\param [inout] ptr The pointer to string data, is advanced one character
\return The character the cursor was pointing to beforce advancing
*/
template <typename T>
inline uint32_t AdvanceCursorFixed(const char*& ptr)
{
	uint32_t out = *ptr;
	++ptr;
	return out;
}

//! Advance a string cursor in an utf8 string
/**
\param [inout] ptr The pointer to string data, is advanced one character
\return The character the cursor was pointing to beforce advancing
*/
inline uint32_t AdvanceCursorUnicode(const char*& ptr)
{
	uint8_t u0 = ptr[0];
	if((u0 & 0x80) == 0) { // 0xxxxxxx 
		ptr += 1;
		return (uint32_t)u0;
	}
	uint8_t u1 = ptr[1];
	if((u0 & 0xE0) == 0xC0) { // 110xxxxx 10xxxxxx
		ptr += 2;
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	}
	uint8_t u2 = ptr[2];
	if((u0 & 0xF0) == 0xE0) { // 1110xxxx 10xxxxxx 10xxxxxx
		ptr += 3;
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	}
	uint8_t u3 = ptr[3];
	if((u0 & 0xF8) == 0xF0) { // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 4;
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC) << 0;
	}

	uint8_t u4 = ptr[4];
	if((u0 & 0xFD) == 0xF8) { // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 6;
		return (u0&~0xFD) << 24 | (u1&~0xC0) << 18 | (u2&~0xC0) << 12 | (u3 &~0xC) << 6 | (u4 &~0xC) << 0;
	}
	uint8_t u5 = ptr[4];
	if((u0 & 0xFE) == 0xFD) { // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 7;
		return (u0&~0xFE) << 30 | (u1&~0xC0) << 24 | (u2&~0xC0) << 18 | (u3 &~0xC) << 12 | (u4 &~0xC) << 6 | (u5 &~0xC) << 0;
	}

	return 0;
}

//! Advance a string cursor in an any type string
/**
\param [inout] ptr The pointer to string data, is advanced one character
\return The character the cursor was pointing to beforce advancing
*/
inline uint32_t AdvanceCursor(StringType type, const char*& ptr)
{
	switch(type) {
	case Unicode:
		return AdvanceCursorUnicode(ptr);
	case Ascii:
		return AdvanceCursorFixed<uint8_t>(ptr);
	case CodePoint:
		return AdvanceCursorFixed<uint32_t>(ptr);
	default:
		throw exception();
	}
}

}
#endif // #ifndef INCLUDED_FORMAT_STRING_BASICS_H
