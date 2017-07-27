#include "stdafx.h"

UNIT_SUITE(algorithm)
{
	UNIT_TEST(sort)
	{
		core::Array<int> arr;
		arr.PushBack(3);
		arr.PushBack(2);
		arr.PushBack(1);
		arr.PushBack(7);
		arr.PushBack(5);
		arr.PushBack(9);
		arr.PushBack(6);
		arr.PushBack(6);
		arr.PushBack(4);
		arr.PushBack(8);
		arr.PushBack(0);

		arr.Sort();

		bool sorted = true;
		for(u32 i = 1; i < arr.Size(); ++i) {
			if(arr[i-1] > arr[i]) {
				sorted = false;
				break;
			}
		}

		UNIT_ASSERT(sorted);
	}

	UNIT_TEST(binary_search)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* it = core::BinarySearch(4, core::MakeRange(array, array+10));

		UNIT_ASSERT(*it == 4);
	}

	UNIT_TEST(linear_search)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* it = core::LinearSearch(4, core::MakeRange(array, array+10));

		UNIT_ASSERT(*it == 4);
	}

	UNIT_TEST(binary_search_failed)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* jt;
		int* it = core::BinarySearch(17, core::MakeRange(array, array+10), &jt);

		UNIT_ASSERT(it == array+10);
		UNIT_ASSERT(jt == array+5);
	}

	UNIT_TEST(binary_search_failed2)
	{
		int array[11] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543, 600};

		int* jt;
		int* it = core::BinarySearch(17, core::MakeRange(array, array+11), &jt);

		UNIT_ASSERT(it == array+11);
		UNIT_ASSERT(jt == array+5);
	}

	UNIT_TEST(linear_search_failed)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* it = core::LinearSearch(17, core::MakeRange(array, array+10));

		UNIT_ASSERT(it == array+10);
	}

}

