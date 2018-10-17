#ifndef INCLUDED_LUX_D3D9_EXCEPTION
#define INCLUDED_LUX_D3D9_EXCEPTION
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "Win32Exception.h"
#include <dxerr/dxerr.h>
#include "core/lxUnicodeConversion.h"

namespace lux
{
namespace core
{

struct D3D9Exception : RuntimeException
{
	static core::ExceptionSafeString MakeErrorString(HRESULT hr)
	{
		core::ExceptionSafeString str;

		str.Append("D3D9 Error(");

		char BUFFER[] = "0x????????";
		char* p = BUFFER+2;
		u32 m = 0xF0000000;
		for(int i = 0; i < 8; ++i) {
			int v = ((hr&m) >> ((7-i)*4));
			*p++ = (char)(v<10?v+'0':v+'A'-10);
			m >>= 4;
		}

		const WCHAR* errstr = DXGetErrorStringW(hr);
		auto utf8ErrStr = core::UTF16ToUTF8(errstr, -1);
		utf8ErrStr.PushBack(0);

		str.Append(BUFFER);
		str.Append("): ");
		str.Append((const char*)utf8ErrStr.Data());
		return str;
	}

	explicit D3D9Exception(HRESULT hr) :
		result(hr)
	{
	}

	ExceptionSafeString What() const { return MakeErrorString(result); }

	HRESULT result;
};

}
}

#endif

#endif // #ifndef INCLUDED_LUX_D3D9_EXCEPTION