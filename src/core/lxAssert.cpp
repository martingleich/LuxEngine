#include "core/LuxBase.h"
#include "core/lxAssert.h"
#include <cstdio>
#include <cstdarg>

namespace lux
{
namespace impl_assert
{

namespace
{
bool DefaultHandler(const char* file, int line, const char* assert_str, const char* msg)
{
	if(file)
		std::printf("%s (%d)", file, line);

	if(assert_str) {
		if(file)
			std::printf(": ");
		std::printf("Assert failed '%s'", assert_str);
	}

	if(msg) {
		if(file || assert_str)
			std::printf(": ");
		std::fputs(msg, stdout);
	}

	std::printf("\n");

	return true;
}

struct HandlingData
{
	AssertHandler handler;
	HandlingData() :
		handler(&DefaultHandler)
	{}
};

HandlingData g_HandlingData;
}


AssertHandler SetHandler(AssertHandler newHandler)
{
	if(!newHandler)
		return nullptr;
	AssertHandler old = g_HandlingData.handler;
	g_HandlingData.handler = newHandler;
	return old;
}

AssertHandler GetHandler()
{
	return g_HandlingData.handler;
}

bool Report(const char* file, int line, const char* assert, const char* msg)
{
	return g_HandlingData.handler(file, line, assert, msg);
}

}
}
