#ifndef INCLUDED_LX_ASSERT_H
#define INCLUDED_LX_ASSERT_H
#include <cstdlib>

namespace lux
{
namespace assert_namespace
{

typedef bool (*AssertHandler)(const char* file, size_t line, const char* assert_msg, const char* msg);

LUX_API AssertHandler SetHandler(AssertHandler newHandler);
LUX_API AssertHandler GetHandler();

LUX_API bool Report(const char* file, size_t line, const char* lxAssert, const char* msg);
}
}

#if defined(_MSC_VER)
#define LUX_DEBUG_BREAK ((void)__debugbreak())
#elif defined(__GNUG__)
#define LUX_DEBUG_BREAK ((void)__builtin_trap())
#else
#define LUX_DEBUG_BREAK ((void)std::abort())
#endif

#ifdef LUX_ENABLE_ASSERTS
#define lxAssert(_Expression)            ((void)(!(_Expression) && ::lux::assert_namespace::Report(__FILE__, __LINE__, #_Expression, nullptr) && (LUX_DEBUG_BREAK, 1)))
#define lxAssertEx(_Expression, _Msg)    ((void)(!(_Expression) && ::lux::assert_namespace::Report(__FILE__, __LINE__, #_Expression, (_Msg)) && (LUX_DEBUG_BREAK, 1)))
#define lxAssertMsg(_Msg)                ((void)(::lux::assert_namespace::Report(__FILE__, __LINE__, nullptr, (_Msg)) && (LUX_DEBUG_BREAK, 1)))
#define lxAssertNeverReach(_Msg)         ((void)(::lux::assert_namespace::Report(__FILE__, __LINE__, nullptr, (_Msg)) && (LUX_DEBUG_BREAK, 1)))
#else
#define lxAssert(_Expression)         ((void)0)
#define lxAssertEx(_Expression, _Msg) ((void)0)
#define lxAssertMsg(_Msg)             ((void)0)
#define lxAssertNeverReach(_Msg)      ((void)0)
#endif

#endif // #ifndef INCLUDED_LX_ASSERT_H
