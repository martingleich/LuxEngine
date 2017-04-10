#include "UnitTestEx.h"

UNIT_SUITE(QuaternionTest)
{
	math::quaternionf q1;

	UNIT_SUITE_FIXTURE_ENTER
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

		UNIT_ASSERT(q1.Equal(x));
	}

	UNIT_TEST(ToAngleAxis)
	{
		math::anglef Angle;
		math::vector3f Axis;
		q1.ToAngleAxis(Angle, Axis);

		Axis.Normalize();

		UNIT_ASSERT(math::IsEqual(Angle.Degree(), 45.0f, 0.005f) && Axis.Equal(math::vector3f(1.0f, 0.0f, 1.0f).Normalize()));
	}

	UNIT_TEST(Transform)
	{
		math::vector3f Point = math::vector3f(1.0f, 0.0f, 0.0f);
		Point = q1.Transform(Point);

		math::vector3f Solution = math::vector3f(0.853553414f, 0.5f, 0.146446615f);

		UNIT_ASSERT(Point.Equal(Solution));
	}

	UNIT_TEST(TransformInPlace)
	{
		math::vector3f Point = math::vector3f(1.0f, 0.0f, 0.0f);
		q1.TransformInPlace(Point);

		math::vector3f Solution = math::vector3f(0.853553414f, 0.5f, 0.146446615f);

		UNIT_ASSERT(Point.Equal(Solution));
	}

	UNIT_TEST(InvTransform)
	{
		math::vector3f Point = math::vector3f(0.853553414f, 0.5f, 0.146446615f);
		Point = q1.TransformInv(Point);

		UNIT_ASSERT(Point.Equal(math::vector3f(1.0f, 0.0f, 0.0f)));
	}

	UNIT_TEST(InvTransformInPlace)
	{
		math::vector3f Point = math::vector3f(0.853553414f, 0.5f, 0.146446615f);
		q1.TransformInPlaceInv(Point);

		UNIT_ASSERT(Point.Equal(math::vector3f(1.0f, 0.0f, 0.0f)));
	}
}