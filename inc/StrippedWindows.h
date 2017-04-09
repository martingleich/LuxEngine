#ifndef INCLUDED_STRIPPED_WINDOWS_H
#define INCLUDED_STRIPPED_WINDOWS_H

#ifndef _INC_WINDOWS
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#undef CreateFile
#undef MoveFile
#undef CopyFile
#undef DeleteFile
#undef Min
#undef Max
#undef min
#undef max

#endif