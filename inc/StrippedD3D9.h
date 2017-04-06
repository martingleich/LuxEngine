#ifndef INCLUDED_STRIPPED_D3D9
#define INCLUDED_STRIPPED_D3D9

#ifndef _D3D9_H_
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include "..\external\d3d9\inc\d3d9.h"
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