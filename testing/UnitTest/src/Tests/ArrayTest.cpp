#include "stdafx.h"

UNIT_SUITE(Array)
{
	struct Type
	{
		Type(int& r, int v) :
			refCount(&r),
			value(v)
		{
			*refCount = *refCount + 1;
		}

		Type(const Type& other) :
			refCount(other.refCount),
			value(other.value)
		{
			*refCount = *refCount + 1;
		}

		Type& operator=(const Type& other)
		{
			*refCount = *refCount - 1;
			refCount = other.refCount;
			value = other.value;
			*refCount = *refCount + 1;
			return *this;
		}

		~Type()
		{
			*refCount = *refCount - 1;
			refCount = nullptr;
		}

		int* refCount;
		int value;
	};

	UNIT_TEST(init)
	{
		core::Array<Type> arr;

		UNIT_ASSERT(arr.IsEmpty());
		UNIT_ASSERT(arr.Size() == 0);
	}

	UNIT_TEST(push_back)
	{
		int refCount1 = 0;
		int refCount2 = 0;
		int refCount3 = 0;

		core::Array<Type> arr;
		arr.PushBack(Type(refCount1, 111));
		arr.PushBack(Type(refCount2, 222));
		arr.PushBack(Type(refCount3, 333));

		UNIT_ASSERT(arr.Size() == 3);
		UNIT_ASSERT(arr[0].value == 111);
		UNIT_ASSERT(arr[1].value == 222);
		UNIT_ASSERT(arr[2].value == 333);

		UNIT_ASSERT(refCount1 == 1);
		UNIT_ASSERT(refCount2 == 1);
		UNIT_ASSERT(refCount3 == 1);
	}

	UNIT_TEST(push_front)
	{
		int refCount1 = 0;
		int refCount2 = 0;
		int refCount3 = 0;

		core::Array<Type> arr;
		arr.PushFront(Type(refCount1, 111));
		arr.PushFront(Type(refCount2, 222));
		arr.PushFront(Type(refCount3, 333));

		UNIT_ASSERT(arr.Size() == 3);
		UNIT_ASSERT(arr[2].value == 111);
		UNIT_ASSERT(arr[1].value == 222);
		UNIT_ASSERT(arr[0].value == 333);

		UNIT_ASSERT(refCount1 == 1);
		UNIT_ASSERT(refCount2 == 1);
		UNIT_ASSERT(refCount3 == 1);
	}

	UNIT_TEST(insert_center)
	{
		int refCount1 = 0;
		int refCount2 = 0;
		int refCount3 = 0;

		core::Array<Type> arr;
		arr.PushBack(Type(refCount1, 111));
		arr.PushBack(Type(refCount2, 222));
		arr.Insert(Type(refCount3, 333), arr.First()+1);

		UNIT_ASSERT(arr.Size() == 3);
		UNIT_ASSERT(arr[0].value == 111);
		UNIT_ASSERT(arr[1].value == 333);
		UNIT_ASSERT(arr[2].value == 222);

		UNIT_ASSERT(refCount1 == 1);
		UNIT_ASSERT(refCount2 == 1);
		UNIT_ASSERT(refCount3 == 1);
	}

	UNIT_TEST(Erase)
	{
		int refCount1 = 0;
		int refCount2 = 0;
		int refCount3 = 0;

		core::Array<Type> arr;
		arr.PushBack(Type(refCount1, 111));
		arr.PushBack(Type(refCount2, 222));
		arr.PushBack(Type(refCount3, 333));

		arr.Erase(core::AdvanceIterator(arr.First(), 1), true);
		UNIT_ASSERT(arr.Size() == 2);
		UNIT_ASSERT(arr[0].value == 111);
		UNIT_ASSERT(arr[1].value == 333);

		UNIT_ASSERT(refCount1 == 1);
		UNIT_ASSERT(refCount2 == 0);
		UNIT_ASSERT(refCount3 == 1);

		arr.Erase(core::AdvanceIterator(arr.First(), 1));
		UNIT_ASSERT(arr.Size() == 1);
		UNIT_ASSERT(arr[0].value == 111);

		UNIT_ASSERT(refCount1 == 1);
		UNIT_ASSERT(refCount2 == 0);
		UNIT_ASSERT(refCount3 == 0);
	}
}
