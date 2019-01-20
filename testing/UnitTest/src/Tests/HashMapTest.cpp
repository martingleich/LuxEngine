#include "stdafx.h"

UNIT_SUITE(HashMap)
{
	u32 permutation[1000];

	UNIT_SUITE_INIT()
	{
		for(int i = 0; i < 1000; ++i)
			permutation[i] = i;

		srand(0);
		for(int i = 0; i < 1000; ++i)
			std::swap(permutation[i], permutation[rand() % 1000]);
	}

	UNIT_TEST(Constructor)
	{
		core::HashMap<u32, u32> map;
		UNIT_ASSERT(map.IsEmpty());
	}

	UNIT_TEST(AddKey)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);
		UNIT_ASSERT(map.Size() == 2);
		u32 test = 0;
		for(auto k : map.Keys())
			if(k == 1)
				test++;
		UNIT_ASSERT(test == 1);
		UNIT_ASSERT(map.At(1) == 2);
		UNIT_ASSERT(map.At(2) == 1);
	}

	UNIT_TEST(TestKey)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		UNIT_ASSERT(map.HasKey(1));
		UNIT_ASSERT(map.HasKey(2));
		UNIT_ASSERT(map.HasKey(3) == false);
	}

	UNIT_TEST(Get)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		UNIT_ASSERT(map.Get(1) == 2);
		UNIT_ASSERT(map.Get(5, 66) == 66);
	}

	UNIT_TEST(ChangeEntry)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		map.At(2) = 3;

		UNIT_ASSERT(map.HasKey(1));
		UNIT_ASSERT(map.HasKey(2));
		UNIT_ASSERT(map.At(2) == 3);
	}

	UNIT_TEST(EraseEntry)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		map.Erase(1);

		UNIT_ASSERT(map.HasKey(1) == false);
		UNIT_ASSERT(map.HasKey(2));
	}

	UNIT_TEST(Clear)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		map.Clear();
		UNIT_ASSERT(map.IsEmpty());
	}

	UNIT_TEST(Copy)
	{
		core::HashMap<u32, u32> mapOriginal;
		mapOriginal.Set(1, 2);
		mapOriginal.Set(2, 1);

		core::HashMap<u32, u32> map = mapOriginal;

		UNIT_ASSERT(map.Size() == 2);
		u32 test = 0;
		for(auto k : map.Keys()) {
			if(k == 1)
				test++;
		}
		UNIT_ASSERT(test == 1);
		UNIT_ASSERT(map.At(1) == 2);
		UNIT_ASSERT(map.At(2) == 1);
	}

	UNIT_TEST(FindKey)
	{
		core::HashMap<u32, u32> map;
		map.Set(1, 2);
		map.Set(2, 1);

		UNIT_ASSERT(map.Find(1) != map.end());
		UNIT_ASSERT(map.Find(2) != map.end());
		UNIT_ASSERT(map.Find(3) == map.end());
	}

	UNIT_TEST(DefaultAdd)
	{
		core::HashMap<u32, u32> map;
		map.At(1, 0);

		UNIT_ASSERT(map[1] == 0);
		UNIT_ASSERT(map.Size() == 1);
		map.At(1, 0) += 5;
		UNIT_ASSERT(map[1] == 5);
		UNIT_ASSERT(map.Size() == 1);
	}

	UNIT_TEST(BigMap)
	{
		core::HashMap<u32, u32> map;
		for(int i = 0; i < 1000; ++i)
			map[i] = i*i;

		bool success = true;
		for(int i = 0; i < 1000; ++i) {
			if(map.Get(permutation[i], 1001) != permutation[i] * permutation[i])
				success = false;
		}
		UNIT_ASSERT(success);
	}
}