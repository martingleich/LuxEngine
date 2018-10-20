#include "core/lxUnicodeConversion.h"
#include "core/lxUnicode.h"

namespace lux
{
namespace core
{

Array<u8> UTF16ToUTF8(const void* _data, int size)
{
	auto data = (const char*)_data;
	auto end = data+size;
	Array<u8> out;
	u8 buffer[4];
	while(data != end) {
		u32 c = AdvanceCursorUTF16(data);
		if(size == -1 && !c)
			break;
		u8* cur = buffer;
		int bytes = CodePointToUTF8(c, cur);
		for(int i = 0; i < bytes; ++i)
			out.PushBack(*cur++);
	}
	return out;
}

core::String UTF16ToString(const void* _data, int size)
{
	auto data = (const char*)_data;
	auto end = data+size;
	core::String out;
	while(data != end) {
		u32 c = AdvanceCursorUTF16(data);
		if(size == -1 && !c)
			break;
		u8 buffer[4];
		u8* cur = buffer;
		int bytes = CodePointToUTF8(c, cur);
		for(int i = 0; i < bytes; ++i)
			out.AppendByte(*cur++);
	}

	return out;
}

Array<u16>& UTF8ToUTF16(const void* _data, int size, Array<u16>& out)
{
	const char* data = (const char*)_data;
	auto end = data+size;
	while(data != end) {
		u32 c = AdvanceCursorUTF8(data);
		if(size == -1 && !c)
			break;
		u16 buffer[2];
		int bytes = CodePointToUTF16(c, buffer);
		out.PushBack(buffer[0]);
		if(bytes == 4)
			out.PushBack(buffer[1]);
	}

	return out;
}

int CodePointToUTF8(u32 c, void* _dst)
{
	u8* start = (u8*)_dst;
	u8* dst = start;
	if(c <= 0x7F) {
		dst[0] = ((u8)c);
		return 1;
	} else if(c <= 0x7FF) {
		dst[0] = ((u8)(0xC0 | ((c&(0x1F << 6)) >> 6)));
		dst[1] = ((u8)(0x80 | ((c & 0x3F))));
		return 2;
	} else if(c <= 0xFFFF) {
		dst[0] = ((u8)(0xE0 | ((c & (0xF << 12)) >> 12)));
		dst[1] = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		dst[2] = ((u8)(0x80 | ((c & 0x3F))));
		return 3;
	} else if(c <= 0x1FFFFF) {
		dst[0] = ((u8)(0xF0 | ((c&(0x7 << 18)) >> 18)));
		dst[1] = ((u8)(0x80 | ((c & (0x3F << 12)) >> 12)));
		dst[2] = ((u8)(0x80 | ((c & (0x3F << 6)) >> 6)));
		dst[3] = ((u8)(0x80 | ((c & 0x3f))));
		return 4;
	} else {
		throw UnicodeException(c);
	}
}

int CodePointToUTF16(u32 c, void* _dst)
{	
	u16* dst = (u16*)_dst;
	if(c < 0xFFFF) {
		dst[0] = (u16)c;
		return 2;
	} else if(c <= 0x1FFFFF) {
		c -= 0x10000;
		u16 h = 0xD800 | (c & 0x3FF);
		uint16_t l = 0xDC00 | ((c >> 10) & 0x3FF);
		dst[0] = h;
		dst[1]= l;
		return 4;
	} else {
		throw UnicodeException(c);
	}
}

}
}