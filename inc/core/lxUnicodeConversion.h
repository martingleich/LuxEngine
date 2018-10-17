#ifndef INCLUDED_LUX_UNICODE_CONVERSION_H
#define INCLUDED_LUX_UNICODE_CONVERSION_H
#include "lxString.h"
#include "lxArray.h"

namespace lux
{
namespace core
{

//! Convert a utf16-string to a array of utf8 elements.
LUX_API Array<u8> UTF16ToUTF8(const void* data, int size);

//! Convert a utf16-string to a string.
LUX_API core::String UTF16ToString(const void* data, int size);

//! Convert a utf8-string to a array of utf16 elements
LUX_API Array<u16>& UTF8ToUTF16(const void* data, int size, Array<u16>& dst);

//! Convert a utf8-string to a array of utf16 elements
LUX_API Array<u16>& UTF8ToUTF16(const void* data, int size, Array<u16>& dst);

struct Win32String
{
public:
	Win32String(Array<u16>&& old) :
		data(old)
	{
	}

	operator const wchar_t*() const { return (const wchar_t*)data.Data(); }
	const void* Data() const { return data.Data(); }
private:
	Array<u16> data;
};

inline Win32String UTF8ToWin32String(const void* data, int size)
{
	Array<u16> out;
	UTF8ToUTF16(data, size, out);
	out.PushBack(0);
	return Win32String(std::move(out));
}
inline Win32String UTF8ToWin32String(StringView view)
{
	return UTF8ToWin32String(view.Data(), view.Size());
}


//! Convert a unicode codepoint to utf-8 data
/*
\param c The character to conver
\param dst Here the utf8 data is written, must be at least 6 byte big.
\return The numer of output bytes
*/
LUX_API int CodePointToUTF8(u32 c, void* dst);

//! Convert a unicode codepoint to utf-16 data.
/*
\param c The character to conver
\param dst Here the utf16 data is written, must be at least 4 byte big.
\return The numer of output bytes
*/
LUX_API int CodePointToUTF16(u32 c, void* dst);

}
}

#endif // #ifndef INCLUDED_LUX_UNICODE_CONVERSION_H
