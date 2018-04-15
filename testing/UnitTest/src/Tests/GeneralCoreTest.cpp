#include "UnitTestEx.h"
#include "core/SafeCast.h"

UNIT_SUITE(GeneralCoreTest)
{
	UNIT_TEST(CheckedCast)
	{
		u64 dstu64;
		u32 dstu32;
		u16 dstu16;
		s64 dsts64;
		s16 dsts16;
		// Unsigned to unsigned
		UNIT_ASSERT(core::CheckedCast((u32)1000, dstu64));
		UNIT_ASSERT(core::CheckedCast((u64)100000000000, dstu32) == false);
		// Signed to unsigned
		UNIT_ASSERT(core::CheckedCast((s32)1000, dstu64)); // Working
		UNIT_ASSERT(core::CheckedCast((s32)100000, dstu16) == false); // Overflow
		UNIT_ASSERT(core::CheckedCast((s32)-100, dstu16) == false); // Negative Overflow

		// Unsigned to Signed
		UNIT_ASSERT(core::CheckedCast((u32)1000, dsts64)); // Working
		UNIT_ASSERT(core::CheckedCast((u32)65535, dsts16) == false); // Overflow, close
		UNIT_ASSERT(core::CheckedCast((u32)100000, dsts16) == false); // Overflow, big
	}
}