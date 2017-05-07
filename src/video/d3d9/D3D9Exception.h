#ifndef INCLUDED_D3D9_EXCEPTION
#define INCLUDED_D3D9_EXCEPTION

#ifdef LUX_COMPILE_WITH_D3D9
#include "core/lxException.h"
#include "LuxEngine/Win32Exception.h"

namespace lux
{
namespace core
{

struct D3D9Exception : RuntimeException
{
	static char* MakeErrorString(HRESULT hr)
	{
		static char error[] = "d3d9 error: ????????";
		char* p = error+12;
		u32 m = 0xF0000000;
		for(int i = 0; i < 8; ++i) {
			int v = ((hr&m) >> ((7-i)*4));
			*p++ = (char)(v<10?v+'0':v+'A'-10);
			m >>= 4;
		}
		return error;
	}

	explicit D3D9Exception(HRESULT hr) :
		RuntimeException(MakeErrorString(hr)),
		result(hr)
	{
	}

	HRESULT result;
};

}
}

#endif

#endif // #ifndef INCLUDED_D3D9_EXCEPTION