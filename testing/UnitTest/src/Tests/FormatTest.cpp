#include "UnitTestEx.h"
#include "../external/format/format/SinkStdString.h"

UNIT_SUITE(format)
{
	UNIT_TEST(GetLocale)
	{
		format::locale::GetLocale();
	}

	UNIT_TEST(IntPrecision)
	{
		std::string str;
		format::format(str, "~.3d", 3);
		UNIT_ASSERT_EQUAL(str, "003");

		format::format(str, "~.3d", 7532);
		UNIT_ASSERT_EQUAL(str, "7532");
	}

	UNIT_TEST(IntToString)
	{
		char buffer[32];
		format::IntToString(0, buffer);
		UNIT_ASSERT_CSTR(buffer, "0");

		format::IntToString(2341, buffer);
		UNIT_ASSERT_CSTR(buffer, "2341");

		format::IntToString(-321, buffer);
		UNIT_ASSERT_CSTR(buffer, "-321");
	}

	UNIT_TEST(FloatToString)
	{
		char buffer[32];
		format::FloatToString(0, buffer);
		UNIT_ASSERT_CSTR(buffer, "0");

		format::FloatToString(2341, buffer);
		UNIT_ASSERT_CSTR(buffer, "2341");

		format::FloatToString(-321.233, buffer);
		UNIT_ASSERT_CSTR(buffer, "-321.233");

		format::FloatToString(321.003, buffer);
		UNIT_ASSERT_CSTR(buffer, "321.003");

		format::FloatToString(0.003, buffer);
		UNIT_ASSERT_CSTR(buffer, "0.003");

		format::FloatToString(-std::numeric_limits<double>::infinity(), buffer);
		UNIT_ASSERT_CSTR(buffer, "-inf");

		format::FloatToString(nan(""), buffer);
		UNIT_ASSERT_CSTR(buffer, "nan");
	}
}