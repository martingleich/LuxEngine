#include "core/lxSTDIO.h"
#include "core/lxUnicodeConversion.h"

namespace lux
{
namespace core
{

FILE* FOpenUTF8(const char* filename, const char* mode)
{
#ifdef LUX_LINUX
	return fopen(filename, mode);
#endif

#ifdef LUX_WINDOWS
	auto path = core::UTF8ToWin32String(filename);
	auto mod = core::UTF8ToWin32String(mode);
	return _wfopen((const wchar_t*)path.Data(), (const wchar_t*)mod.Data());
#endif
}

}
}
