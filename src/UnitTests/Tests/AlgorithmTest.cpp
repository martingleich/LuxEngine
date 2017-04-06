#include "UnitTestEx.h"

UNIT_SUITE(algorithm)
{
	UNIT_TEST(sort)
	{
		core::array<int> arr;
		arr.Push_Back(3);
		arr.Push_Back(2);
		arr.Push_Back(1);
		arr.Push_Back(7);
		arr.Push_Back(5);
		arr.Push_Back(9);
		arr.Push_Back(6);
		arr.Push_Back(6);
		arr.Push_Back(4);
		arr.Push_Back(8);
		arr.Push_Back(0);

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

		int* it = core::Binary_Search(4, array, array+10);

		UNIT_ASSERT(*it == 4);
	}

	UNIT_TEST(linear_search)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* it = core::Linear_Search(4, array, array+10);

		UNIT_ASSERT(*it == 4);
	}

	UNIT_TEST(binary_search_failed)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* jt;
		int* it = core::Binary_Search(17, array, array+10, &jt);

		UNIT_ASSERT(it == array+10);
		UNIT_ASSERT(jt == array+5);
	}

	UNIT_TEST(binary_search_failed2)
	{
		int array[11] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543, 600};

		int* jt;
		int* it = core::Binary_Search(17, array, array+11, &jt);

		UNIT_ASSERT(it == array+11);
		UNIT_ASSERT(jt == array+5);
	}

	UNIT_TEST(linear_search_failed)
	{
		int array[10] = {1, 3, 4,6, 8, 32, 46, 122, 467, 543};

		int* it = core::Linear_Search(17, array, array+10);

		UNIT_ASSERT(it == array+10);
	}

}

