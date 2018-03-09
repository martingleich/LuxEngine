#ifndef INCLUDED_FORMAT_STRING_BASICS_H
#define INCLUDED_FORMAT_STRING_BASICS_H
#include "format/Exception.h"
#include <inttypes.h>

namespace format
{

//! Returns the number of codepoints in the string, null-termainted
inline size_t StringLength(const char* ptr)
{
	size_t len = 0;
	const uint8_t* s = (const uint8_t*)ptr;
	while(*s) {
		if((*s & 0xC0) != 0x80)
			++len;
		++s;
	}

	return len;
}

//! Advance a string cursor and return current codepoint.
/**
\param [inout] ptr The pointer to string data, is advanced one character
\return The character the cursor was pointing to beforce advancing
*/
inline uint32_t AdvanceCursor(const char*& ptr)
{
	uint8_t u0 = *ptr++;
	if((u0 & 0x80) == 0) // 0xxxxxxx 
		return (uint32_t)u0;
	uint8_t u1 = *ptr++;
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	uint8_t u2 = *ptr++;
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	uint8_t u3 = *ptr++;
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC0) << 0;

	return 0;
}

//! Read a single character
inline uint32_t GetCharacter(const char* ptr)
{
	uint8_t u0 = *ptr++;
	if((u0 & 0x80) == 0) // 0xxxxxxx 
		return (uint32_t)u0;
	uint8_t u1 = *ptr++;
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	uint8_t u2 = *ptr++;
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	uint8_t u3 = *ptr++;
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC0) << 0;
	return 0;
}

}

#endif // #ifndef INCLUDED_FORMAT_STRING_BASICS_H
