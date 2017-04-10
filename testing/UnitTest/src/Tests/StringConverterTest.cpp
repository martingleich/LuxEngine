#include "UnitTestEx.h"

UNIT_SUITE(ConvertTest)
{}

UNIT_SUITE(ParseTest)
{
	UNIT_TEST(Integer)
	{
		int value;
		value = lux::core::StringConverter::ParseInt("192742");
		UNIT_ASSERT(value == 192742);

		value = lux::core::StringConverter::ParseInt("-14");
		UNIT_ASSERT(value == -14);

		value = lux::core::StringConverter::ParseInt("+64");
		UNIT_ASSERT(value == 64);

		value = lux::core::StringConverter::ParseInt("abc24");
		UNIT_ASSERT(value == 0);

		value = lux::core::StringConverter::ParseInt("234abc");
		UNIT_ASSERT(value == 234);
	}
	UNIT_TEST(Float)
	{
		float value;
		value = lux::core::StringConverter::ParseFloat("1234");
		UNIT_ASSERT(math::IsEqual(value, 1234.0f));

		value = lux::core::StringConverter::ParseFloat("1234.5678");
		UNIT_ASSERT(math::IsEqual(value, 1234.5678f));
	}
}