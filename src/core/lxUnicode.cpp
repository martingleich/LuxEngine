#include "core/lxUnicode.h"
#include "core/lxException.h"

extern "C"
{
#include "../external/utf8proc/utf8proc.h"
}

namespace lux
{
namespace core
{

size_t StringLengthUTF8(const char* str)
{
	size_t len = 0;
	const u8* s = (const u8*)str;
	while(*s) {
		if((*s & 0xC0) != 0x80)
			++len;
		++s;
	}

	return len;
}

void RetractCursorUTF8(const char*& ptr)
{
	--ptr;
	while((*ptr & 0xC0) == 0x80)
		--ptr;
}

u32 AdvanceCursorUTF8(const char*& ptr)
{
	u8 u0 = ptr[0];
	if((u0 & 0x80) == 0) { // 0xxxxxxx 
		ptr += 1;
		return (u32)u0;
	}
	u8 u1 = ptr[1];
	if((u0 & 0xE0) == 0xC0) { // 110xxxxx 10xxxxxx
		ptr += 2;
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	}
	u8 u2 = ptr[2];
	if((u0 & 0xF0) == 0xE0) { // 1110xxxx 10xxxxxx 10xxxxxx
		ptr += 3;
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	}
	u8 u3 = ptr[3];
	if((u0 & 0xF8) == 0xF0) { // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 4;
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC) << 0;
	}
	u8 u4 = ptr[4];
	if((u0 & 0xFD) == 0xF8) { // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 5;
		return (u0&~0xFD) << 24 | (u1&~0xC0) << 18 | (u2&~0xC0) << 12 | (u3 &~0xC) << 6 | (u4 &~0xC) << 0;
	}
	u8 u5 = ptr[4];
	if((u0 & 0xFE) == 0xFD) { // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		ptr += 6;
		return (u0&~0xFE) << 30 | (u1&~0xC0) << 24 | (u2&~0xC0) << 18 | (u3 &~0xC) << 12 | (u4 &~0xC) << 6 | (u5 &~0xC) << 0;
	}

	throw UnicodeException("Invalid utf8 codepoint");
}

u32 GetCharacterUTF8(const char* ptr)
{
	u8 u0 = ptr[0];
	if((u0 & 0x80) == 0) // 0xxxxxxx
		return (u32)u0;
	u8 u1 = ptr[1];
	if((u0 & 0xE0) == 0xC0) // 110xxxxx 10xxxxxx
		return (u0&~0xE0) << 6 | (u1&~0xC0);
	u8 u2 = ptr[2];
	if((u0 & 0xF0) == 0xE0) // 1110xxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF0) << 12 | (u1&~0xC0) << 6 | (u2&~0xC0) << 0;
	u8 u3 = ptr[3];
	if((u0 & 0xF8) == 0xF0) // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xF8) << 18 | (u1&~0xC0) << 12 | (u2&~0xC0) << 6 | (u3 &~0xC) << 0;
	u8 u4 = ptr[4];
	if((u0 & 0xFD) == 0xF8) // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xFD) << 24 | (u1&~0xC0) << 18 | (u2&~0xC0) << 12 | (u3 &~0xC) << 6 | (u4 &~0xC) << 0;
	u8 u5 = ptr[4];
	if((u0 & 0xFE) == 0xFD) // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		return (u0&~0xFE) << 30 | (u1&~0xC0) << 24 | (u2&~0xC0) << 18 | (u3 &~0xC) << 12 | (u4 &~0xC) << 6 | (u5 &~0xC) << 0;

	throw UnicodeException("Invalid utf8 codepoint");
}

size_t StringLengthUTF16(const char* str)
{
	size_t length = 0;
	while(AdvanceCursorUTF16(str))
		++length;
	return length;
}

u32 AdvanceCursorUTF16(const char*& _ptr)
{
	const u8* ptr = (const u8*)_ptr;
	u16 h = ((u16)ptr[0] | ((u16)ptr[1] << 8));
	if(h <= 0xD7FF || h >= 0xE000) {
		_ptr += 2;
		return h;
	}
	
	h &= ~0xFC00;
	u16 l = (ptr[2] | ptr[3] << 8);
	l &= ~0xFC00;

	u32 out = ((h << 10) | l) + 0x10000;

	_ptr += 4;
	return out;
}

u32 GetCharacterUTF16(const char* _ptr)
{
	const u8* ptr = (const u8*)_ptr;
	u16 h = ((u16)ptr[0] | ((u16)ptr[1] << 8));
	if(h <= 0xD7FF || h >= 0xE000) {
		_ptr += 2;
		return h;
	}
	
	h &= ~0xFC00;
	u16 l = (ptr[2] | ptr[3] << 8);
	l &= ~0xFC00;

	u32 out = ((h << 10) | l) + 0x10000;

	return out;
}

bool IsEqualCaseInsensitive(u32 a, u32 b)
{
	// TODO: This is wrong. Use case folding to do this
	return utf8proc_tolower(a) == utf8proc_tolower(b);
}

u32 ToLowerChar(u32 c)
{
	return utf8proc_tolower(c);
}

u32 ToUpperChar(u32 c)
{
	return utf8proc_toupper(c);
}

bool IsDigit(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_ND;
}

bool IsSpace(u32 c)
{
	auto cat = utf8proc_category(c);
	return c == '\n' || c == '\r' || c == ' ' || c == '\t' || cat == UTF8PROC_CATEGORY_ZS || cat == UTF8PROC_CATEGORY_ZL || cat == UTF8PROC_CATEGORY_ZP;
}

bool IsAlpha(u32 c)
{
	auto cat = utf8proc_category(c);
	return (cat == UTF8PROC_CATEGORY_LU ||
			cat == UTF8PROC_CATEGORY_LL ||
			cat == UTF8PROC_CATEGORY_LT ||
			cat == UTF8PROC_CATEGORY_LM ||
			cat == UTF8PROC_CATEGORY_LO);
}

bool IsUpper(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_LU;
}

bool IsLower(u32 c)
{
	return utf8proc_category(c) == UTF8PROC_CATEGORY_LL;
}
}
}