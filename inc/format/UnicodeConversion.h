#ifndef INCLUDED_FORMAT_UNICODE_CONVERSION_H
#define INCLUDED_FORMAT_UNICODE_CONVERSION_H
#include <vector>

namespace format
{

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

inline void CodePointToUtf16(uint32_t c, std::vector<uint16_t>& out)
{
	if(c < 0xFFFF) {
		out.push_back((uint16_t)c);
	} else {
		c -= 0x10000;
		uint16_t c1 = 0xD800 | (c & 0x3FF);
		uint16_t c2 = 0xDC00 | ((c >> 10) & 0x3FF);
		out.push_back(c1);
		out.push_back(c2);
	}
}

inline void Utf8ToUtf16(const char* data, size_t size, std::vector<uint16_t>& out)
{
	// Approximation for new size.
	out.reserve(out.size() + size);

	while(size) {
		--size;
		uint32_t c = AdvanceCursor(data);
		CodePointToUtf16(c, out);
	}
}

}
#endif // #ifndef INCLUDED_FORMAT_UNICODE_CONVERSION_H
