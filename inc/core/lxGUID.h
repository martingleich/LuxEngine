#ifndef INCLUDED_LUX_GUID_H
#define INCLUDED_LUX_GUID_H
#include <cstring>
#include "core/LuxBase.h"
#include "core/lxFormat.h"

namespace lux
{
namespace core
{

class GUID
{
public:
	LUX_API static GUID EMPTY;

	GUID()
	{
		memset(bytes, 0, 16);
	}

	static GUID FromBytes(const void* bytes)
	{
		GUID out;
		memcpy(out.bytes, bytes, 16);
		return out;
	}

	static GUID FromString(const core::String& str)
	{
		return FromString(str.Data(), str.Size());
	}

	static GUID FromString(const char* str, int length = -1)
	{
		if(length < 0)
			length = strlen(str);
		if(length != 36)
			return GUID::EMPTY;
		GUID out;
		u8* bytes = out.bytes;
		for(int i = 0; i < 4; ++i) {
			*bytes++ = GetByte(str);
			str += 2;
		}

		for(int j = 0; j < 3; ++j) {
			if(*str != '-')
				return GUID::EMPTY;
			++str;
			for(int i = 0; i < 2; ++i) {
				*bytes++ = GetByte(str);
				str += 2;
			}
		}

		if(*str != '-')
			return GUID::EMPTY;
		++str;
		for(int i = 0; i < 6; ++i) {
			*bytes++ = GetByte(str);
			str += 2;
		}
		return out;
	}

	bool operator==(const GUID& other) const
	{
		return memcmp(bytes, other.bytes, 16) == 0;
	}
	bool operator!=(const GUID& other) const
	{
		return !(*this == other);
	}

	const u8* Bytes() const
	{
		return bytes;
	}

private:
	static u8 GetNibble(const char c)
	{
		if(c >= '0' && c <= '9')
			return c - '0';
		if(c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if(c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return 0;
	}
	static u8 GetByte(const char* str)
	{
		return (GetNibble(str[0]) << 4) | GetNibble(str[1]);
	}

private:
	u8 bytes[16];
};

LUX_API void fmtPrint(format::Context& ctx, const core::GUID& g, format::Placeholder& placeholder);

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_GUID_H