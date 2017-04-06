#include "core/lxUnicodeConversion.h"
#include "core/lxUnicode.h"

namespace lux
{
namespace core
{

u16* CodePointToUTF16(u32 c, u16* dst)
{	
	u16* out = (u16*)dst;
	if(c < 0xFFFF) {
		*out++ = (u16)c;
	} else {
		c -= 0x10000;
		u16 h = 0xD800 | (c & 0x3FF);
		uint16_t l = 0xDC00 | ((c >> 10) & 0x3FF);
		*out++ = h;
		*out++= l;
	}

	return (u16*)out;
}

array<u8> UTF16ToUTF8(const void* _data)
{
	const char* data = (const char*)_data;
	array<u8> out;
	while(u32 c = AdvanceCursorUTF16(data)) {
		u8 buffer[6];
		u8* cur = buffer;
		u8* end = CodePointToUTF8(c, cur);
		while(cur < end)
			out.Push_Back(*cur++);
	}

	out.Push_Back(0);

	return out;
}

string UTF16ToString(const void* _data)
{
	const char* data = (const char*)_data;
	string out;
	while(u32 c = AdvanceCursorUTF16(data)) {
		u8 buffer[6];
		u8* cur = buffer;
		u8* end = CodePointToUTF8(c, cur);
		while(cur < end)
			out.PushByte(*cur++);
	}

	return out;
}

array<u16> UTF8ToUTF16(const void* _data)
{
	const char* data = (const char*)_data;
	array<u16> out;
	while(u32 c = AdvanceCursorUTF8(data)) {
		u16 buffer[2];
		u16* cur = buffer;
		u16* end = CodePointToUTF16(c, cur);
		while(cur < end)
			out.Push_Back(*cur++);
	}

	out.Push_Back(0);

	return out;
}

WCharAlias UTF8ToUTF16W(const void* data)
{
	return WCharAlias(UTF8ToUTF16(data));
}

WCharAlias StringToUTF16W(const string& data)
{
	return WCharAlias(UTF8ToUTF16(data.Data_c()));
}

u8* CodePointToUTF8(u32 c, u8* dst)
{
	u8* orig = dst;

	if(c <= 0x7F) {
		*dst++ = ((u8)c);
	} else if(c <= 0x7FF) {
		*dst++ = ((u8)(0xC0 | ((c&(0x1F << 6)) >> 6)));
		*dst++ = ((u8)(0x80 | ((c & 0x3F))));
	} else if(c <= 0xFFFF) {
		*dst++ = ((u8)(0xE0 | ((c & (0xF << 12)) >> 12)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*dst++ = ((u8)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x1FFFFF) {
		*dst++ = ((u8)(0xF0 | ((c&(0x7 << 18)) >> 18)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*dst++ = ((u8)(0x80 | ((c & 0x3f))));
	} else if(c <= 0x3FFFFFF) {
		*dst++ = ((u8)(0xF8 | ((c&(0x3 << 24)) >> 24)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*dst++ = ((u8)(0x80 | ((c & 0x3F))));
	} else if(c <= 0x7FFFFFFF) {
		*dst++ = ((u8)(0xFC | ((c&(0x1 << 30)) >> 30)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 24)) >> 24)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 18)) >> 18)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 12)) >> 12)));
		*dst++ = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		*dst++ = ((u8)(0x80 | ((c & 0x3F))));
	} else {
		assert(false);
	}

	return dst;
}

u8* CodePointToUTF16(u32 c, u8* dst)
{	
	u16* out = (u16*)dst;
	if(c < 0xFFFF) {
		*out++ = (u16)c;
	} else {
		c -= 0x10000;
		u16 h = 0xD800 | (c & 0x3FF);
		uint16_t l = 0xDC00 | ((c >> 10) & 0x3FF);
		*out++ = h;
		*out++= l;
	}

	return (u8*)out;
}

}
}