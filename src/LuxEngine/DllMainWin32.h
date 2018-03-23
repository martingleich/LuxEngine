#ifndef INCLUDED_LUX_DLL_MAIN_WIN32_H
#define INCLUDED_LUX_DLL_MAIN_WIN32_H
#ifdef LUX_WINDOWS
#include "core/LuxBase.h"
#include "platform/StrippedWindows.h"

namespace lux
{
HINSTANCE GetLuxModule();
}

#endif // LUX_WINDOWS

#endif // #ifndef INCLUDED_LUX_DLL_MAIN_WIN32_H