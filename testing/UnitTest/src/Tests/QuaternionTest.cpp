#include "stdafx.h"

UNIT_SUITE(QuaternionTest)
{
	math::quaternionf q1;

	UNIT_SUITE_FIXTURE_ENTER()
	{
		q1.x = 0.2705980500730984921998616026832f;
		q1.y = 0.0f;
		q1.z = 0.2705980500730984921998616026832f;
		q1.w = 0.92387953251128675612818318939679f;
	}

	UNIT_TEST(FromAngleAxis)
	{
		math::anglef Angle = math::anglef::Degree(45.0f);

		math::quaternionf x;
		x = math::quaternionf::FromAngleAxis(Angle, math::vector3f(1.0f, 0.0f, 1.0f).Normalize());

		UNIT_ASSERT_APPROX(q1, x);
	}

	UNIT_TEST(ToAngleAxis)
	{
		math::anglef Angle;
		math::vector3f Axis;
		q1.ToAngleAxis(Angle, Axis);

		Axis.Normalize();

		UNIT_ASSERT(math::IsEqual(Angle.Degree(), 45.0f, 0.005f) && math::IsEqual(Axis, math::vector3f(1.0f, 0.0f, 1.0f).Normalize()));
	}

	UNIT_TEST(ToEuler)
	{
		// TODO: Do the real test
		auto q = math::quaternionf::FromEuler(math::vector3f(1.0f, 2.0f, 3.0f));

		auto v = q.ToEuler();

		LUX_UNUSED(v);
	}

	UNIT_TEST(Transform)
	{
		math::vector3f Point = math::vector3f(1.0f, 0.0f, 0.0f);
		Point = q1.Transform(Point);

		math::vector3f Solution = math::vector3f(0.853553414f, 0.5f, 0.146446615f);

		UNIT_ASSERT_APPROX(Point, Solution);
	}

	UNIT_TEST(TransformInPlace)
	{
		math::vector3f Point = math::vector3f(1.0f, 0.0f, 0.0f);
		q1.TransformInPlace(Point);

		math::vector3f Solution = math::vector3f(0.853553414f, 0.5f, 0.146446615f);

		UNIT_ASSERT_APPROX(Point, Solution);
	}

	UNIT_TEST(InvTransform)
	{
		math::vector3f Point = math::vector3f(0.853553414f, 0.5f, 0.146446615f);
		Point = q1.TransformInv(Point);

		UNIT_ASSERT_APPROX(Point, math::vector3f(1.0f, 0.0f, 0.0f));
	}

	UNIT_TEST(InvTransformInPlace)
	{
		math::vector3f Point = math::vector3f(0.853553414f, 0.5f, 0.146446615f);
		q1.TransformInPlaceInv(Point);

		UNIT_ASSERT_APPROX(Point, math::vector3f(1.0f, 0.0f, 0.0f));
	}
}
