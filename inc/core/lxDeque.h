#ifndef INCLUDED_LUX_DEQUE_H
#define INCLUDED_LUX_DEQUE_H
#include "core/LuxBase.h"

namespace lux
{
namespace core
{

template <typename T>
class Deque
{
	static const int BLOCK_SIZE = 16;
private:
	struct Block
	{
		T data[BLOCK_SIZE];
		static Block* Alloc() { return (Block*)::operator new(sizeof(Block)); }
		static void Free(Block* b) { ::operator delete(b); }
	};

	/*
	A list of pointers to blocks
	|block|block|block|block|block|block|
	Used blocks are placed in the middle of the list so it can grow into both directions.
	|free|free|allocated|allocated|allocated|free|
	*/
	class BaseList
	{
	public:
		BaseList() :
			m_RawFirst(nullptr),
			m_RawEnd(nullptr),
			m_DataFirst(nullptr),
			m_DataEnd(nullptr)
		{
		}

		BaseList(BaseList&& old) :
			m_RawFirst(old.m_RawFirst),
			m_RawEnd(old.m_RawEnd),
			m_DataFirst(old.m_DataFirst),
			m_DataEnd(old.m_DataEnd)
		{
			old.m_RawFirst = old.m_RawEnd =
				old.m_DataFirst = old.m_DataEnd = nullptr;
		}

		~BaseList()
		{
			for(auto it = m_RawFirst; it != m_RawEnd; ++it)
				Block::Free(*it);

			delete[] m_RawFirst;
		}

		void AddFront()
		{
			if(m_DataFirst == m_RawFirst) {
				Balance(true);
				if(m_DataFirst == m_RawFirst)
					Resize(2 * Allocated(), true);
			}
			--m_DataFirst;
		}

		void AddBack()
		{
			if(m_DataEnd == m_RawEnd) {
				Balance(false);
				if(m_DataEnd == m_RawEnd)
					Resize(2 * Allocated(), false);
			}
			++m_DataEnd;
		}

		void RemoveFront() { ++m_DataFirst; }
		void RemoveBack() { --m_DataEnd; }

		int Allocated() const { return m_RawEnd - m_RawFirst; }
		int Size() const { return m_DataEnd - m_DataFirst; }

		void Resize(int newAlloc, bool front)
		{
			/*
			Reserve more blocks.
			\param newCount The new number of blocks.
			\param front If true enfore at least on member in the front, otherwise enfore at least one at the back.
			*/
			int used_count = m_DataEnd - m_DataFirst;
			int alloc_count = m_RawEnd - m_RawFirst;
			Block** newRaw = new Block*[newAlloc];
			int allocDataOffset = m_DataFirst - m_RawFirst;
			Block** newDataFirst = newRaw + ((newAlloc - used_count) / 2);
			if(front) {
				if(newRaw == newDataFirst)
					newDataFirst++;
			}

			// Copy the old allocated blocks.
			if(alloc_count) {
				std::memcpy(newDataFirst - allocDataOffset, m_RawFirst, alloc_count * sizeof(Block*));
				delete[] m_RawFirst;
			}
			// Alloc the new blocks.
			for(auto it = newRaw; it != newDataFirst - allocDataOffset; ++it)
				*it = Block::Alloc();
			for(auto it = newDataFirst - allocDataOffset + alloc_count; it != newRaw + newAlloc; ++it)
				*it = Block::Alloc();

			// Fill in members
			m_RawFirst = newRaw;
			m_RawEnd = m_RawFirst + newAlloc;
			m_DataFirst = newDataFirst;
			m_DataEnd = m_DataFirst + used_count;
			if(used_count == 0)
				++m_DataEnd;
		}

		Block** First() { return m_DataFirst; }
		Block** First() const { return m_DataFirst; }
		Block** Last() { return m_DataEnd - 1; }
		Block** Last() const { return m_DataEnd - 1; }

		void Balance(bool hint)
		{
			LUX_UNUSED(hint);
			/*
			Shift the datarange into the middle of the rawrange.
			*/
			intptr_t left = (intptr_t)(m_DataFirst - m_RawFirst);
			intptr_t right = (intptr_t)(m_RawEnd - m_DataEnd);
			intptr_t shift = (right - left) / 2;
			if(shift) {
				// Swap blocks
				int count = int(m_DataEnd - m_DataFirst);
				if(shift < 0) {
					auto firstSrc = m_DataFirst;
					auto firstDst = firstSrc + shift;
					for(int i = 0; i < count; ++i) {
						std::swap(*firstSrc, *firstDst);
						++firstSrc;
						++firstDst;
					}
				} else {
					auto lastSrc = m_DataEnd - 1;
					auto lastDst = lastSrc + shift;
					for(int i = 0; i < count; ++i) {
						std::swap(*lastSrc, *lastDst);
						--lastSrc;
						--lastDst;
					}
				}
				m_DataFirst += shift;
				m_DataEnd += shift;
			}
		}

		void Ensure()
		{
			/*
			Ensure at least one block.
			*/
			if(!m_RawFirst)
				Resize(1, false);
		}

		// TODO: Only alloc Blocks if necessary.
	private:
		Block** m_RawFirst;
		Block** m_DataFirst;
		Block** m_DataEnd;
		Block** m_RawEnd;
	};

public:
	Deque() :
		m_FirstId(BLOCK_SIZE / 2),
		m_EndId(BLOCK_SIZE / 2)
	{
	}

	void Reserve(int allocated)
	{
		m_Base.Resize((allocated + BLOCK_SIZE - 1) / BLOCK_SIZE, true);
	}

	Deque(Deque&& old) :
		m_Base(std::move(old.m_Base)),
		m_FirstId(old.m_FirstId),
		m_EndId(old.m_EndId)
	{
		old.m_FirstId = BLOCK_SIZE / 2;
		old.m_EndId = BLOCK_SIZE / 2;
	}

	Deque(const Deque& other) :
		m_FirstId(BLOCK_SIZE / 2),
		m_EndId(BLOCK_SIZE / 2)
	{
		m_Base.Resize(other.m_Base.Size());

		for(auto& x : other)
			PushBack(x);
	}

	~Deque()
	{
		Clear();
	}

	Deque& operator=(const Deque& other)
	{
		Clear();
		m_FirstId = BLOCK_SIZE / 2;
		m_EndId = BLOCK_SIZE / 2;
		if(m_Base.Size() < other.m_Base.Size())
			m_Base.Resize(other.m_Base.Size());

		for(auto& x : other)
			PushBack(x);
		return *this;
	}

	Deque& operator=(Deque&& old)
	{
		Clear();
		m_Base = BaseList(std::move(old.m_Base));
		m_FirstId = old.m_FirstId;
		m_EndId = old.m_EndId;
		old.m_FirstId = BLOCK_SIZE / 2;
		old.m_EndId = BLOCK_SIZE / 2;
		return *this;
	}

	void Clear()
	{
		int i = Size();
		while(i != 0) {
			PopBack();
			--i;
		}

		m_Base.Balance(true);
		m_FirstId = BLOCK_SIZE / 2;
		m_EndId = BLOCK_SIZE / 2;
	}

	void PushFront(const T& elem) { new (NewFrontPtr()) T(elem); }
	void PushFront(T&& elem) { new (NewFrontPtr()) T(std::move(elem)); }

	template <typename... Ts>
	void EmplaceFront(Ts&&... args) { new (NewFrontPtr()) T(std::forward<Ts>(args)...); }

	void PushBack(const T& elem) { new (NewBackPtr()) T(elem); }
	void PushBack(T&& elem) { new (NewBackPtr()) T(std::move(elem)); }

	template <typename... Ts>
	void EmplaceBack(Ts&&... args) { new (NewBackPtr()) T(std::forward<Ts>(args)...); }

	void PopFront()
	{
		Front().~T();
		++m_FirstId;
		if(m_FirstId == BLOCK_SIZE) {
			m_Base.RemoveFront();
			m_FirstId = 0;
		}
	}

	void PopBack()
	{
		Back().~T();
		--m_EndId;
		if(m_EndId == 0) {
			m_Base.RemoveBack();
			m_EndId = BLOCK_SIZE;
		}
	}

	int Size() const
	{
		if(m_Base.Size() == 0)
			return 0;
		return BLOCK_SIZE * (m_Base.Size() - 1) + (m_EndId - m_FirstId);
	}

	bool IsEmpty() const { return Size() == 0; }

	const T& Front() const { return (*m_Base.First())->data[m_FirstId]; }
	T& Front() { return (*m_Base.First())->data[m_FirstId]; }
	const T& Back() const { return (*m_Base.Last())->data[m_EndId - 1]; }
	T& Back() { return (*m_Base.Last())->data[m_EndId - 1]; }

	const T& At(int id) const
	{
		auto start = m_Base.First();
		auto absId = m_FirstId + id;
		return start[absId / BLOCK_SIZE]->data[absId%BLOCK_SIZE];
	}
	T& At(int id)
	{
		auto start = m_Base.First();
		auto absId = m_FirstId + id;
		return start[absId / BLOCK_SIZE]->data[absId%BLOCK_SIZE];
	}
private:
	T* NewFrontPtr()
	{
		m_Base.Ensure();
		if(m_FirstId == 0) {
			m_Base.AddFront();
			m_FirstId = BLOCK_SIZE - 1;
		} else {
			--m_FirstId;
		}
		return (*m_Base.First())->data + m_FirstId;
	}

	T* NewBackPtr()
	{
		m_Base.Ensure();
		if(m_EndId == BLOCK_SIZE) {
			m_Base.AddBack();
			m_EndId = 1;
		} else {
			++m_EndId;
		}
		return (*m_Base.Last())->data + (m_EndId - 1);
	}

private:
	BaseList m_Base;
	int m_FirstId;
	int m_EndId;
};


} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_DEQUE_H