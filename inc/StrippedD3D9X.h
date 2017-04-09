#ifndef INCLUDED_STRIPPED_D3D9X_H
#define INCLUDED_STRIPPED_D3D9X_H

#ifndef __D3DX9_H__
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include "..\external\d3d9\inc\d3dx9.h"
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