#pragma once
#include "StringType.h"
#include "Exception.h"
#include <cinttypes>
#include <vector>

namespace format
{
template <typename T>
inline size_t StringLengthFixed(const char* str)
{
	const T* s = (const T*)str;
	size_t len = 0;
	while(*s++)
		++len;
	return len;
}

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

// Returns the number of codepoints in the string.
inline size_t StringLength(StringType type, const char* str)
{
	switch(type) {
	case StringType::Unicode:
		return StringLengthUnicode(str);
	case StringType::Ascii:
		return StringLengthFixed<uint8_t>(str);
	case StringType::CodePoint:
		return StringLengthFixed<uint32_t>(str);
	default:
		throw exception();
	}
}

inline char* AppendStr(char* dst, const char* src)
{
	const uint8_t* s = (const uint8_t*)src;
	uint8_t* d = (uint8_t*)dst;
	while(*s)
		*d++ = *s++;

	return (char*)d;
}

template <typename T>
inline uint32_t GetCharacterFixed(const char* ptr)
{
	return static_cast<T>(*(reinterpret_cast<const T*>(ptr)));
}

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

inline uint32_t GetCharacter(StringType type, const char* str)
{
	switch(type) {
	case StringType::Unicode:
		return GetCharacterUnicode(str);
	case StringType::Ascii:
		return GetCharacterFixed<uint8_t>(str);
	case StringType::CodePoint:
		return GetCharacterFixed<uint32_t>(str);
	default:
		throw exception();
	}
}

template <typename T>
inline uint32_t AdvanceCursorFixed(const char*& ptr)
{
	uint32_t out = *ptr;
	++ptr;
	return out;
}

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

inline uint32_t AdvanceCursor(StringType type, const char*& ptr)
{
	switch(type) {
	case StringType::Unicode:
		return AdvanceCursorUnicode(ptr);
	case StringType::Ascii:
		return AdvanceCursorFixed<uint8_t>(ptr);
	case StringType::CodePoint:
		return AdvanceCursorFixed<uint32_t>(ptr);
	default:
		throw exception();
	}
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
	} else {
		assert(false);
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
		assert(false);
	}

	return (int)(out-orig);
}

inline std::vector<uint16_t> CodePointsToUtf16(const uint32_t* data, size_t count)
{
	std::vector<uint16_t> utf16;
	utf16.reserve(count);

	const uint32_t* end = data + count;
	for(auto it = data; *it && it != end; ++it)
		CodePointToUtf16(*it, utf16);

	return utf16;
}

inline std::vector<uint16_t> Utf8ToUtf16(const char* data, size_t size)
{
	std::vector<uint16_t> utf16;
	utf16.reserve(size / 2);

	while(size) {
		uint32_t c = AdvanceCursorUnicode(data);
		--size;
		CodePointToUtf16(c, utf16);
	}

	return utf16;
}


}