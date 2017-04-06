#ifndef INCLUDED_LX_UNICODE_CONVERSION
#define INCLUDED_LX_UNICODE_CONVERSION
#include "lxString.h"
#include "lxArray.h"

namespace lux
{
namespace core
{

//! Helper type to cast from array<u16> to const wchar_t*
struct WCharAlias
{
	WCharAlias(const array<u16>& d) :
		data(std::move(d))
	{}
	array<u16> data;

	operator const wchar_t*()
	{
		return (const wchar_t*)data.Data_c();
	}
};

//! Convert a nulterminated utf16-string to a array of utf8 elements(nulterminated)
array<u8> UTF16ToUTF8(const void* data);

//! Convert a nulterminated utf16-string to a string.
string UTF16ToString(const void* data);

//! Convert a nulterminated utf8-string to a array of utf16 elements(nulterminated)
array<u16> UTF8ToUTF16(const void* data);

//! Convert a nulterminated utf8-string to a array of utf16 elements(nulterminated).
/**
The return value of this method can be cast to const wchar_t*.
It can be used inside a funktion call, receiving a wchar_t* pointer.
*/
WCharAlias UTF8ToUTF16W(const void* data);

//! Convert a string to a array of utf16 elements(nulterminated).
/**
The return value of this method can be cast to const wchar_t*.
It can be used inside a funktion call, receiving a wchar_t* pointer.
*/
WCharAlias StringToUTF16W(const string& data);

//! Convert a unicode codepoint to utf-8 data
/*
\param c The character to conver
\param dst Here the utf8 data is written, must be at least 6 byte big.
\return A pointer to the byte after the last one containing valid data.
*/
u8* CodePointToUTF8(u32 c, u8* dst);

//! Convert a unicode codepoint to utf-16 data.
/*
\param c The character to conver
\param dst Here the utf16 data is written, must be at least 4 byte big.
\return A pointer to the byte after the last one containing valid data.
*/
u8* CodePointToUTF16(u32 c, u8* dst);

//! Convert a unicode codepoint to utf-16 data.
/*
\param c The character to conver
\param dst Here the utf16 data is written, must be at least 4 byte big.
\return A pointer to the byte after the last one containing valid data.
*/
u16* CodePointToUTF16(u32 c, u16* dst);

}
}

#endif // #ifndef INCLUDED_LX_UNICODE_CONVERSION