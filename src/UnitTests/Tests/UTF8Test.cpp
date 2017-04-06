#include "UnitTestEx.h"

UNIT_SUITE(unicode)
{
	UNIT_TEST(AdvanceUTF8Test)
	{
		const char* str = "a☠b";
		const char* end = str + strlen(str);
		u32 a = core::AdvanceCursorUTF8(str);
		u32 p = core::AdvanceCursorUTF8(str);
		u32 b = core::AdvanceCursorUTF8(str);

		UNIT_ASSERT_EQUAL(a, 'a');
		UNIT_ASSERT_EQUAL(p, 0x2620);
		UNIT_ASSERT_EQUAL(b, 'b');
		UNIT_ASSERT_EQUAL(str, end);
	}

	UNIT_TEST(GetUTF8Test)
	{
		const char* str = "a☠";
		u32 a = core::GetCharacterUTF8(str);
		u32 p = core::GetCharacterUTF8(str+1);

		UNIT_ASSERT_EQUAL(a, 'a');
		UNIT_ASSERT_EQUAL(p, 0x2620);
	}

	UNIT_TEST(AdvanceUTF16Test)
	{
		const char* str = (const char*)u"\U00024B62\U000020AC\U00000024";
		const char* end = str + 8;
		u32 a = core::AdvanceCursorUTF16(str);
		u32 b = core::AdvanceCursorUTF16(str);
		u32 c = core::AdvanceCursorUTF16(str);

		UNIT_ASSERT_EQUAL(a, 0x24B62);
		UNIT_ASSERT_EQUAL(b, 0x20AC);
		UNIT_ASSERT_EQUAL(c, 0x24);
		UNIT_ASSERT_EQUAL(str, end);
	}

	UNIT_TEST(StringLengthUTF16Test)
	{
		const char* str = (const char*)u"\U00024B62\U000020AC\U00000024";
		size_t length = core::StringLengthUTF16(str);

		UNIT_ASSERT_EQUAL(length, 3);
	}
}