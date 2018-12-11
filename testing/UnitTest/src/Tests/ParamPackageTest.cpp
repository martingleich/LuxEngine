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
		UNIT_ASSERT(puffer.FromName("arg1_int", true).Get<int>() == 1);
		UNIT_ASSERT(puffer.FromName("arg2_float", true).Get<float>() == 1.0f);
		UNIT_ASSERT(puffer.FromName("arg3_bool", true).Get<bool>() == false);
		UNIT_ASSERT(puffer.FromName("arg4_string", true).Get<core::StringView>().Equal("Hello"));
	}
}

