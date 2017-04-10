#include "UnitTestEx.h"

UNIT_SUITE(format)
{
	UNIT_TEST(GetLocale)
	{
		format::locale::GetLocale();
	}
}