#ifndef INCLUDED_FORMAT_CONFIG_H
#define INCLUDED_FORMAT_CONFIG_H
#include "core/LuxBase.h"

#ifdef LUX_EXPORT
#define FORMAT_API __declspec(dllexport)
#else
#define FORMAT_API __declspec(dllimport)
#define LUX_TEMPLATE_EXTERN extern
#endif

#ifdef LUX_WINDOWS
#define FORMAT_WINDOWS
#endif

#endif // #ifndef INCLUDED_FORMAT_CONFIG_H