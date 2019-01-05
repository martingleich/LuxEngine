#include "stdafx.h"
#include "format/sinks/SinkStdString.h"
#include "format/sinks/SinkCString.h"

UNIT_SUITE(format)
{
	UNIT_TEST(GetLocale)
	{
		format::GetLocale();
	}

	UNIT_TEST(IntPrecision)
	{
		std::string str;
		format::format(str, "{!d:.3}", 3);
		UNIT_ASSERT_EQUAL(str, "003");

		format::format(str, "{!d:.3}", 7532);
		UNIT_ASSERT_EQUAL(str, "7532");
	}

	UNIT_TEST(IntToString)
	{
		char buffer[32];
		format::format(buffer, "{}", 0);
		UNIT_ASSERT_CSTR(buffer, "0");

		format::format(buffer, "{}", 2341);
		UNIT_ASSERT_CSTR(buffer, "2341");

		format::format(buffer, "{}", -321);
		UNIT_ASSERT_CSTR(buffer, "-321");
	}

	UNIT_TEST(FloatToString)
	{
		char buffer[32];
		format::format(buffer, "{}", 0.0f);
		UNIT_ASSERT_CSTR(buffer, "0");

		format::format(buffer, "{}", 2341.0f);
		UNIT_ASSERT_CSTR(buffer, "2341");

		format::format(buffer, "{}", -321.233f);
		UNIT_ASSERT_CSTR(buffer, "-321.233");

		format::format(buffer, "{}", 321.003f);
		UNIT_ASSERT_CSTR(buffer, "321.003");

		format::format(buffer, "{}", 0.003f);
		UNIT_ASSERT_CSTR(buffer, "0.003");

		format::format(buffer, "{}", -std::numeric_limits<double>::infinity());
		UNIT_ASSERT_CSTR(buffer, "-inf");

		format::format(buffer, "{}", nan(""));
		UNIT_ASSERT_CSTR(buffer, "nan");
	}
}