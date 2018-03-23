#ifndef INCLUDED_LUX_STRIPPED_WINDOWS_H
#define INCLUDED_LUX_STRIPPED_WINDOWS_H

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef CreateFile
#undef MoveFile
#undef CopyFile
#undef DeleteFile
#undef Min
#undef Max
#undef min
#undef max

#endif