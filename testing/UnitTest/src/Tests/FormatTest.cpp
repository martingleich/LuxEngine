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
}