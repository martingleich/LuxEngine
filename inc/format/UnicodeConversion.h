#ifndef INCLUDED_FORMAT_UNICODE_CONVERSION_H
#define INCLUDED_FORMAT_UNICODE_CONVERSION_H
#include "format/StringBasics.h"
#include <vector>

namespace format
{
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

inline void CodePointToUtf8(uint32_t c, std::vector<uint8_t>& out)
{
	if(c <= 0x7F) {
		out.push_back((uint8_t)c);
	} else if(c <= 0x7FF) {
		out.push_back((uint8_t)(0xC0 | ((c&(0x1F << 6)) >> 6)));
		out.push_back((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0xFFFF) {
		out.push_back((uint8_t)(0xE0 | ((c & (0xF << 12)) >> 12)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		out.push_back((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x1FFFFF) {
		out.push_back((uint8_t)(0xF0 | ((c&(0x7 << 18)) >> 18)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		out.push_back((uint8_t)(0x80 | ((c & 0x3f))));
	} else if(c <= 0x3FFFFFF) {
		out.push_back((uint8_t)(0xF8 | ((c&(0x3 << 24)) >> 24)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		out.push_back((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x7FFFFFFF) {
		out.push_back((uint8_t)(0xFC | ((c&(0x1 << 30)) >> 30)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 24)) >> 24)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		out.push_back((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		out.push_back((uint8_t)(0x80 | ((c & 0x3F))));
	}
}

inline int CodePointToUtf8(uint32_t c, uint8_t* out)
{
	uint8_t* orig = out;
	if(c <= 0x7F) {
		*out++ = ((uint8_t)c);
	} else if(c <= 0x7FF) {
		*out++ = ((uint8_t)(0xC0 | ((c&(0x1F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0xFFFF) {
		*out++ = ((uint8_t)(0xE0 | ((c & (0xF << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x1FFFFF) {
		*out++ = ((uint8_t)(0xF0 | ((c&(0x7 << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3f))));
	} else if(c <= 0x3FFFFFF) {
		*out++ = ((uint8_t)(0xF8 | ((c&(0x3 << 24)) >> 24)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x7FFFFFFF) {
		*out++ = ((uint8_t)(0xFC | ((c&(0x1 << 30)) >> 30)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 24)) >> 24)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*out++ = ((uint8_t)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*out++ = ((uint8_t)(0x80 | ((c & 0x3F))));
	} else {
		*out++ = 0;
	}

	return (int)(out - orig);
}

inline void CodePointsToUtf16(const uint32_t* data, size_t count, std::vector<uint16_t>& out)
{
	out.reserve(out.size() + count);

	const uint32_t* end = data + count;
	for(auto it = data; *it && it != end; ++it)
		CodePointToUtf16(*it, out);
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
