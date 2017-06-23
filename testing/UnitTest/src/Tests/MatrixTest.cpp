#include "stdafx.h"

UNIT_SUITE(MatrixTest)
{
	UNIT_TEST(Constructor)
	{
		math::matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		bool compare = true;
		float m[4][4] = {
			{1, 2, 3, 4},
			{5, 6, 7, 8},
			{9, 10, 11, 12},
			{13, 14, 15, 16}};

		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c) {
				if(m[r][c] != a(r, c))
					compare = false;
			}
		}

		UNIT_ASSERT(compare);
	}

	UNIT_TEST(Multiply)
	{
		math::matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);
		math::matrix4 b(
			17, 18, 19, 20,
			21, 22, 23, 24,
			25, 26, 27, 28,
			29, 30, 31, 32);
		math::matrix4 result(
			250, 260, 270, 280,
			618, 644, 670, 696,
			986, 1028, 1070, 1112, 
			1354, 1412, 1470, 1528);

		UNIT_ASSERT_APPROX(a*b, result);
	}
}
