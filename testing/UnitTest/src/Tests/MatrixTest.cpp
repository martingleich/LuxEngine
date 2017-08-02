#include "stdafx.h"

UNIT_SUITE(MatrixTest)
{
	UNIT_TEST(BasicConstructor)
	{
		math::Matrix4 a(
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

	UNIT_TEST(Access)
	{
		bool compare = true;
		math::Matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		// Check the () operator
		compare = true;
		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c)
				if(a(r, c) != (float)(r * 4 + c + 1))
					compare = false;
		}
		UNIT_ASSERT(compare);

		// Check the [] operator
		compare = true;
		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c)
				if(a[r * 4 + c] != a(r, c))
					compare = false;
		}
		UNIT_ASSERT(compare);

		// Check the DataRowMajor() function
		compare = true;
		const float *row = a.DataRowMajor();
		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c)
				if(row[r*4+c] != a(r, c))
					compare = false;
		}
		UNIT_ASSERT(compare);
	}

	UNIT_TEST(StaticMembers)
	{
		bool compare = true;

		// Validate the ZERO matrix
		compare = true;
		for(int r = 0; r < 3; ++r) {
			for(int c = 0; c < 3; ++c)
				if(math::Matrix4::ZERO(r, c) != 0.0f)
					compare = false;
		}
		UNIT_ASSERT(compare);

		// Validate the IDENT matrix
		compare = true;
		for(int r = 0; r < 3; ++r) {
			for(int c = 0; c < 3; ++c)
				if(r != c) {
					if(math::Matrix4::IDENTITY(r, c) != 0.0f)
						compare = false;
				} else {
					if(math::Matrix4::IDENTITY(r, c) != 1.0f)
						compare = false;
				}
		}
		UNIT_ASSERT(compare);
	}

	UNIT_TEST(Compare)
	{
		math::Matrix4 a(
			FLT_MIN, -100.50f, 10.0f, -23.838922f,
			FLT_MAX, 9999.9f, 1.0f, 3.333333333f,
			81823.246573554525f, 5380.0f, -243245.1f, 42.424242f,
			249.0f, 0.0f, 91.42f, 12.4f);

		math::Matrix4 b(
			FLT_MIN, -100.50f, 10.0f, -23.838922f,
			FLT_MAX, 9999.9f, 1.0f, 3.333333333f,
			81823.246573554525f, 5380.0f, -243245.1f, 42.424242f,
			249.0f, 0.0f, 91.42f, 12.4f);

		// Validate by hand
		bool compare = true;
		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c) {
				// There should be no rounding errors
				if(a(r, c) != b(r, c))
					compare = false;
			}
		}
		UNIT_ASSERT(compare);

		// Validate with the approx macro
		UNIT_ASSERT_APPROX(a, b);
	}

	UNIT_TEST(Add)
	{
		math::Matrix4 a(
			FLT_MIN, -100.50f, 10.0f, 23.0f, 
			FLT_MAX, 9999.9f, 1.0f, 3.3f, 
			81823.0f, 5380.0f, -243245.1f, 42.424242f,
			249.0f, 0.0f, 91.42f, 12.4f);
		math::Matrix4 b(
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f);

		UNIT_ASSERT_APPROX(a + b, a);

		b = math::Matrix4(
			-FLT_MIN, 100.50f, -10.0f, -23.0f,
			-FLT_MAX, -9999.9f, -1.0f, -3.3f,
			-81823.0f, -5380.0f, 243245.1f, -42.424242f,
			-249.0f, -0.0f, -91.42f, -12.4f);

		UNIT_ASSERT_APPROX(a + b, math::Matrix4::ZERO);
	}

	UNIT_TEST(Subtract)
	{
		math::Matrix4 a(
			FLT_MIN, -100.50f, 10.0f, 23.0f,
			FLT_MAX, 9999.9f, 1.0f, 3.3f,
			81823.0f, 5380.0f, -243245.1f, 42.424242f,
			249.0f, 0.0f, 91.42f, 12.4f);
		math::Matrix4 b(
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f);

		UNIT_ASSERT_APPROX(a - b, a);
		UNIT_ASSERT_APPROX(a - a, math::Matrix4::ZERO);

		b = math::Matrix4(
			-10.0f, 0.50f, 10.0f, 20.0f,
			99999.99f, 199.9f, -142.0f, 32.3f,
			81003.0f, 5380.75f, -21245.1f, 42.424241f,
			1249.0f, 0.0f, 91.42f, 412.4f);

		// Precision error for c[0][0], c[1][0] and c[2][3]
		math::Matrix4 c(
			FLT_MIN + 10.0f, -101.0f, 0.0f, 3.0f,
			FLT_MAX - 99999.99f, 9800.0f, 143.0f, -29.0f,
			820.0f, -0.75f, -222000.0f, 42.424242f - 42.424241f,
			-1000.0f, 0.0f, 0.0f, -400.0f);

		UNIT_ASSERT_APPROX(a - b, c);
	}

	UNIT_TEST(Multiply)
	{
		math::Matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);
		math::Matrix4 b(
			17, 18, 19, 20,
			21, 22, 23, 24,
			25, 26, 27, 28,
			29, 30, 31, 32);
		math::Matrix4 result(
			250, 260, 270, 280,
			618, 644, 670, 696,
			986, 1028, 1070, 1112, 
			1354, 1412, 1470, 1528);

		UNIT_ASSERT_APPROX(a*b, result);
	}

	UNIT_TEST(Functions)
	{
		bool compare;
		math::Matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		math::Matrix4 b;

		// Check GetInverted()
		UNIT_ASSERT(a.GetInverted(b) == false);
		a(2, 1) = 20;
		// TODO This should not work!!!
		UNIT_ASSERT(a.GetInverted(b));
		compare = true;
		// TODO Get a working 4x4 matrix ...
//		for(int r = 0; r < 4; ++r) {
//			for(int c = 0; c < 4; ++c) {
//				if(...)
//					compare = false;
//			}
//		}
//		UNIT_ASSERT(compare);

		// Check GetTransposed()
		a.GetTransposed(b);
		compare = true;
		for(int r = 0; r < 4; ++r) {
			for(int c = 0; c < 4; ++c) {
				if(a(r, c) != b(c, r))
					compare = false;
			}
		}
		UNIT_ASSERT(compare);

		// Check IsIdent
		b = math::Matrix4(1.001f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.001f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.001f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.001f);
		UNIT_ASSERT(b.IsIdent() == false);
		UNIT_ASSERT(math::Matrix4::IDENTITY.IsIdent());
		b.MakeIdent();
		UNIT_ASSERT(b.IsIdent());
		UNIT_ASSERT_APPROX(b, math::Matrix4::IDENTITY);

		// Check the 3x3
		a.Get3x3(b);
		compare = true;
		{
			// Check the top left 2x2
			for(int r = 0; r < 2; ++r) {
				for(int c = 0; c < 2; ++c) {
					if(a(r, c) != b(r, c))
						compare = false;
				}
			}
			// The last row (index 3) should now be the third row (index 2)
			for(int i = 0; i < 2; i++) {
				if(b(i, 2) != a(i, 3))
					compare = false;
				if(b(2, i) != a(3, i))
					compare = false;
			}
			if(b(2, 2) != a(3, 3))
				compare = false;
			// Check the last row 
			for(int i = 0; i < 3; i++) {
				if(b(i, 3) != 0.0f)
					compare = false;
				if(b(3, i) != 0.0f)
					compare = false;
			}
			if(b(3, 3) != 1.0f)
				compare = false;
		}
		UNIT_ASSERT(compare);
	}

	UNIT_TEST(AdvancedConstructor)
	{
		bool compare = true;
		math::Matrix4 a(
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			13, 14, 15, 16);

		math::Matrix4 b(a, lux::math::Matrix4::EMatrix4Constructor::M4C_NOTHING);
		compare = true;
		for(int r = 0; r < 3; ++r) {
			for(int c = 0; c < 3; ++c)
				if(b(r, c) != 0.0f)
					compare = false;
		}
		// TODO How to check this ...
		//UNIT_ASSERT(compare);

		math::Matrix4 d(a, lux::math::Matrix4::EMatrix4Constructor::M4C_IDENT);
		UNIT_ASSERT(d.IsIdent());

		compare = true;
		for(int r = 0; r < 3; ++r) {
			for(int c = 0; c < 3; ++c) {
				if(r != c) {
					if(d(r, c) != 0.0f)
						compare = false;
				} else {
					if(d(r, c) != 1.0f)
						compare = false;
				}
			}
		}
		UNIT_ASSERT(compare);

		math::Matrix4 e(a, lux::math::Matrix4::EMatrix4Constructor::M4C_COPY);
		compare = true;
		for(int r = 0; r < 3; ++r) {
			for(int c = 0; c < 3; ++c)
				if(a(r, c) != e(r, c))
					compare = false;
		}
		UNIT_ASSERT(compare);

		a(2, 1) = 20;
		math::Matrix4 f(a, lux::math::Matrix4::EMatrix4Constructor::M4C_INV);
		UNIT_ASSERT(a.GetInverted(b));
		UNIT_ASSERT_APPROX(f, b);

		math::Matrix4 g(a, lux::math::Matrix4::EMatrix4Constructor::M4C_TRANSP);
		a.GetTransposed(b);
		UNIT_ASSERT_APPROX(g, b);

		math::Matrix4 h(a, lux::math::Matrix4::EMatrix4Constructor::M4C_INV_TRANSP);
		UNIT_ASSERT(a.GetInverted(b));
		b.GetTransposed(d);
		UNIT_ASSERT_APPROX(h, d);
	}
}
