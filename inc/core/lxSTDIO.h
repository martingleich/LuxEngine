#ifndef INCLUDED_LUX_STDIO_H
#define INCLUDED_LUX_STDIO_H
#include <cstdio>

namespace lux
{
namespace core
{
FILE* FOpenUTF8(const char* filename, const char* mode);
}
}

#endif // #ifndef INCLUDED_LUX_STDIO_H
