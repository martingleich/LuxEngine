#include "stdafx.h"

UNIT_SUITE(algorithm)
{
	core::ParamPackage pack;
	UNIT_SUITE_INIT()
	{
		core::ParamPackageBuilder ppb;
		ppb.AddParam("arg1_int", 1);
		ppb.AddParam("arg2_float", 1.0f);
		ppb.AddParam("arg3_bool", false);
		ppb.AddParam("arg4_string", core::String("Hello"));
		pack = ppb.BuildAndReset();
	}
	UNIT_TEST(default_values_and_access)
	{
		core::PackagePuffer puffer(&pack);
		UNIT_ASSERT((int)puffer.FromName("arg1_int", true) == 1);
		UNIT_ASSERT((float)puffer.FromName("arg2_float", true) == 1.0f);
		UNIT_ASSERT((bool)puffer.FromName("arg3_bool", true) == false);
		UNIT_ASSERT(((core::StringView)puffer.FromName("arg4_string", true)).Equal("Hello"));
	}
}

