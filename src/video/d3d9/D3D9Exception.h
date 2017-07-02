#ifndef INCLUDED_D3D9_EXCEPTION
#define INCLUDED_D3D9_EXCEPTION

#ifdef LUX_COMPILE_WITH_D3D9
#include "Win32Exception.h"
#include "../external/dxerr/dxerr.h"
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
		auto utf8ErrStr = core::UTF16ToUTF8(errstr);

		str.Append(BUFFER);
		str.Append("): ");
		str.Append((const char*)utf8ErrStr.Data());
		return str;
	}

	explicit D3D9Exception(HRESULT hr) :
		RuntimeException(nullptr),
		result(hr)
	{
		m_What = MakeErrorString(hr);
	}

	HRESULT result;
};

}
}

#endif

#endif // #ifndef INCLUDED_D3D9_EXCEPTION