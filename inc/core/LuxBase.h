#ifndef INCLUDED_LUXBASE_H
#define INCLUDED_LUXBASE_H

//---------------------------------------------
// PLATFORM
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define LUX_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif

#if !defined(LUX_WINDOWS)
#define LUX_LINUX
#endif

//---------------------------------------------
// LUX_EXPORT
#ifdef LUX_WINDOWS
#ifdef LUX_EXPORT
#define LUX_API __declspec(dllexport)
#define LUX_TEMPLATE_EXTERN
#else
#define LUX_API __declspec(dllimport)
#define LUX_TEMPLATE_EXTERN extern
#endif

#else   // Für nicht Windowssystem

#if (__GNUC__ >= 4) && defined(LUX_EXPORT)
#define LUX_API __attribute__ ((visibility("default")))
#else
#define LUX_API
#endif
#endif

// Löst einen Haltepunkt aus
#include <assert.h>
#undef assert

#ifdef NDEBUG
#define assert(_Expression)         ((void)0)
#define assertEx(_Expression, _Msg) ((void)0)
#define assertMsg(_Msg)             ((void)0)
#define assertNeverReach(_Msg)      ((void)0)
#else  /* NDEBUG */
#ifdef WIN32
#define LUX_DEBUG_BREAK (__debugbreak(),0)
#else
#define LUX_DEBUG_BREAK 0
#endif

#define assert(_Expression)            (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), LUX_DEBUG_BREAK) )
#define assertEx(_Expression, _Msg)    (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(_Msg),         _CRT_WIDE(__FILE__), __LINE__), LUX_DEBUG_BREAK) )
#define assertMsg(_Msg)                (void)(                      (_wassert(_CRT_WIDE(_Msg),         _CRT_WIDE(__FILE__), __LINE__), LUX_DEBUG_BREAK)
#define assertNeverReach(_Msg)                                      (_wassert(_CRT_WIDE(_Msg),         _CRT_WIDE(__FILE__), __LINE__), LUX_DEBUG_BREAK)
#endif

#include <utility>
#include <cstdint>

namespace lux
{
typedef std::uint8_t  u8;    //!< A type for unsigned 8 bit integers
typedef std::uint16_t u16;   //!< A type for unsigned 16 bit integers
typedef std::uint32_t u32;   //!< A type for unsigned 32 bit integers
typedef std::int8_t   s8;    //!< A type for signed 8 bit integers
typedef std::int16_t  s16;   //!< A type for signed 16 bit integers
typedef std::int32_t  s32;   //!< A type for signed 32 bit integers
}

// Define-Funktionen
#define LX_MAKE_FOURCC(c0, c1, c2, c3) ((u32)(c0) | (u32)(c1 << 8) | (u32)(c2 << 16) | (u32)(c3 << 24))

#define LUX_UNUSED(var) ((void)(var))


#ifdef _MSC_VER
#define ifconst(cond) \
__pragma(warning(suppress: 4127)) \
if(cond)
#else
#define ifconst(cond) if(cond)
#endif

#include "core/EnumClassFlags.h"

#include "core/lxMemoryAlloc.h"

#endif