#ifndef INCLUDED_LX_STDIO_H
#define INCLUDED_LX_STDIO_H
#include <cstdio>

namespace lux
{
namespace core
{
FILE* FOpenUTF8(const char* filename, const char* mode);
}
}

#endif // #ifndef INCLUDED_LX_STDIO_H