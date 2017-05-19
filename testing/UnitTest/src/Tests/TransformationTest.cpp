#include "UnitTestEx.h"

using namespace lux;

UNIT_SUITE(TransformationTest)
{
    UNIT_SUITE_DEPEND_ON(QuaternionTest);

	math::Transformation t1;
	math::Transformation t2;

	UNIT_SUITE_FIXTURE_ENTER()
	{
		t1.translation = math::vector3f(2.0f, 3.0, 1.0f);
		t1.scale = 2.0f;
		// Drehe um 1,0,1 um 45�
		t1.orientation.x = 0.2705980500730984921998616026832f;
		t1.orientation.y = 0.0f;
		t1.orientation.z = 0.2705980500730984921998616026832f;
		t1.orientation.w = 0.92387953251128675612818318939679f;

		t2.translation = math::vector3f(-2.0f, 3.0f, 1.0f);
		t2.scale = 3.0f;
		t2.orientation = math::quaternionf::FromAngleAxis(math::anglef::Degree(60.0f), math::vector3f(1.0f, 2.0f, -2.0f));
	}

	UNIT_TEST(ToMatrix)
	{
		math::matrix4 m1, m2;
		m1.BuildWorld(math::vector3f(t1.scale), t1.orientation, t1.translation);
		t1.ToMatrix(m2);

		UNIT_ASSERT_APPROX(m1, m2);
	}

	UNIT_TEST(ToInvMatrix)
	{
		math::matrix4 m1, m2;
		m1.BuildWorld(math::vector3f(t1.scale), t1.orientation, t1.translation).Invert();
		t1.ToMatrixInv(m2);

		UNIT_ASSERT_APPROX(m1, m2);
	}

	UNIT_TEST(TransformPoint)
	{
		math::vector3f a = math::vector3f(1.0f, 2.0f, 3.0f);
		// scale => 2, 4, 6
		// rotate => 0.585786462 0.828427076 7.41421366
		// translate => 2.585786462 3.828427076 8.41421366
		math::vector3f b = math::vector3f(2.585786462f, 3.828427076f, 8.41421366f);
		math::vector3f x = t1.TransformPoint(a);

		UNIT_ASSERT_APPROX(x, b);
	}

	UNIT_TEST(TransformInvPoint)
	{
		math::vector3f a = math::vector3f(2.585786462f, 3.828427076f, 8.41421366f);
		math::vector3f b = math::vector3f(1.0f, 2.0f, 3.0f);
		math::vector3f x = t1.TransformInvPoint(a);

		UNIT_ASSERT_APPROX(x, b);
	}

	UNIT_TEST(Invert)
	{
		math::Transformation tinv;
		t1.GetInverted(tinv);

		math::vector3f a = math::vector3f(2.585786462f, 3.828427076f, 8.41421366f);
		a = tinv.TransformPoint(a);
		math::vector3f b = math::vector3f(1.0f, 2.0f, 3.0f);

		UNIT_ASSERT_APPROX(a, b);
	}

	UNIT_TEST(CombineRight)
	{
		math::vector3f a = math::vector3f(1.0f, 2.0f, 3.0f);
		math::vector3f x;
		x = t1.TransformPoint(a);
		x = t2.TransformPoint(x);
		math::vector3f b = t1.CombineRight(t2).TransformPoint(a);

		// Relativ viele Rechungen darum h�here Abweichung m�glich
		UNIT_ASSERT(math::IsEqual(x, b, 0.0001f));
	}

	UNIT_TEST(CombineLeft)
	{
		math::vector3f a = math::vector3f(1.0f, 2.0f, 3.0f);
		math::vector3f x;
		x = t2.TransformPoint(a);
		x = t1.TransformPoint(x);
		math::vector3f b = t1.CombineLeft(t2).TransformPoint(a);

		// Relativ viele Rechungen darum h�here Abweichung m�glich
		UNIT_ASSERT(math::IsEqual(x, b, 0.0001f));
	}
}