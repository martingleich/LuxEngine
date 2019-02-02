#include "stdafx.h"
#include "core/lxDeque.h"

UNIT_SUITE(Deque)
{
	UNIT_TEST(Constructor)
	{
		core::Deque<int> dq;
		UNIT_ASSERT(dq.IsEmpty());
	}

	UNIT_TEST(UseAsQueue)
	{
		core::Deque<int> dq;
		for(int i = 0; i < 100; ++i)
			dq.PushBack(i);
		int v = 0;
		bool valid=true;
		for(int i = 100; i < 1000; ++i) {
			dq.PushBack(i);
			int k = dq.Front();
			if(k != v)
				valid = false;
			++v;
			dq.PopFront();
		}
		UNIT_ASSERT(valid);
	}
	UNIT_TEST(UseAsQueueR)
	{
		core::Deque<int> dq;
		for(int i = 0; i < 100; ++i)
			dq.PushFront(i);
		int v = 0;
		bool valid=true;
		for(int i = 100; i < 1000; ++i) {
			dq.PushFront(i);
			int k = dq.Back();
			if(k != v)
				valid = false;
			++v;
			dq.PopBack();
		}
		UNIT_ASSERT(valid);
	}
	UNIT_TEST(UseAsStack)
	{
		core::Deque<int> dq;
		for(int i = 0; i < 100; ++i)
			dq.PushBack(i);
		bool valid=true;
		for(int i = 99; i >= 0; --i) {
			int k = dq.Back();
			if(i != k)
				valid = false;
			dq.PopBack();
		}
		UNIT_ASSERT(valid);
	}
	UNIT_TEST(UseAsStackR)
	{
		core::Deque<int> dq;
		for(int i = 0; i < 100; ++i)
			dq.PushFront(i);
		bool valid=true;
		for(int i = 99; i >= 0; --i) {
			int k = dq.Front();
			if(i != k)
				valid = false;
			dq.PopFront();
		}
		UNIT_ASSERT(valid);
	}

	UNIT_TEST(RepeatedQueue)
	{
		core::Deque<int> dq;
		bool isValid = true;
		for(int i = 0; i < 8; ++i) {
			for(int j = 0; j < 4; ++j)
				dq.PushBack(j);
			for(int j = 0; j < 4; ++j) {
				int k = dq.Front();
				if(k != j)
					isValid = false;
				dq.PopFront();
			}
			if(!dq.IsEmpty())
				isValid = false;
		}
		UNIT_ASSERT(isValid);
	}
}
