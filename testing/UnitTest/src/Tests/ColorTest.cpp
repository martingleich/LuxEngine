#include "UnitTestEx.h"

// TODO gcc -Wunknown-pragmas
#ifndef LUX_LINUX
#pragma warning(disable: 4244)
#endif

UNIT_SUITE(Color)
{
	UNIT_TEST(HSV_To_RGB)
	{
		float r,g,b;
		video::HSVToRGB(311.0f/360.0f, 0.5f, 0.67f, r, g, b);
		
		UNIT_ASSERT(math::IsEqual<int>(r*255, 171, 1));
		UNIT_ASSERT(math::IsEqual<int>(g*255, 85, 1));
		UNIT_ASSERT(math::IsEqual<int>(b*255, 155, 1));
	}

	UNIT_TEST(RGB_To_HSV)
	{
		float h,s,v;
		video::RGBToHSV(21.0f/255.0f, 9.0f/255.0f, 90/255.0f, h, s, v);
		
		UNIT_ASSERT(math::IsEqual<int>(h*360, 249, 1));
		UNIT_ASSERT(math::IsEqual<int>(s*100, 90, 1));
		UNIT_ASSERT(math::IsEqual<int>(v*100, 35, 1));
	}
}
