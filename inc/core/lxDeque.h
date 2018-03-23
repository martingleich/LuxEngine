#ifndef INCLUDED_LUX_DEQUE_H
#define INCLUDED_LUX_DEQUE_H
#include "core/LuxBase.h"
#include "core/lxIterator.h"

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
		static Block* Alloc()
		{
			return (Block*)::operator new(sizeof(Block));
		}

		static void Free(Block* b)
		{
			::operator delete(b);
		}
	};

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

			LUX_FREE_ARRAY(m_RawFirst);
		}

		void AddFront()
		{
			lxAssert(m_RawFirst);
			if(m_DataFirst == m_RawFirst)
				Resize(2 * Size(), true);
			--m_DataFirst;
		}

		void AddBack()
		{
			lxAssert(m_RawFirst);
			if(m_DataEnd == m_RawEnd)
				Resize(2 * Size(), false);
			++m_DataEnd;
		}

		void RemoveFront()
		{
			lxAssert(m_RawFirst);
			++m_DataFirst;
		}

		void RemoveBack()
		{
			lxAssert(m_RawFirst);
			--m_DataEnd;
		}

		int Size() const
		{
			return m_RawEnd - m_RawFirst;
		}

		void Resize(int newCount, bool front)
		{
			int count = m_RawEnd - m_RawFirst;
			Block** newData = LUX_NEW_ARRAY(Block*, newCount);
			Block** newFirst = newData + ((newCount - count) / 2);
			if(front) {
				if(newFirst == newData)
					newFirst++;
			}
			if(count) {
				memcpy(newFirst, m_DataFirst, count * sizeof(Block*));
				LUX_FREE_ARRAY(m_RawFirst);
			}
			for(auto it = newData; it != newFirst; ++it)
				*it = Block::Alloc();
			for(auto it = newFirst + count; it != newData + newCount; ++it)
				*it = Block::Alloc();
			m_RawFirst = newData;
			m_RawEnd = m_RawFirst + newCount;
			m_DataFirst = newFirst;
			m_DataEnd = m_DataFirst + count;

			Balance();
		}

		Block** First() { return m_DataFirst; }
		Block** First() const { return m_DataFirst; }
		Block** Last() { return m_DataEnd - 1; }
		Block** Last() const { return m_DataEnd - 1; }

		void Balance()
		{
			lxAssert(m_RawFirst);
			intptr_t left = (intptr_t)(m_DataFirst - m_RawFirst);
			intptr_t right = (intptr_t)(m_RawEnd - m_DataEnd);

			intptr_t shift = (right - left) / 2;
			if(shift) {
				memmove(m_RawFirst + left + shift, m_DataFirst, (m_DataEnd - m_DataFirst) * sizeof(Block*));
				m_DataFirst += shift;
				m_DataEnd += shift;
			}
		}

		void Ensure()
		{
			if(!m_RawFirst) {
				Resize(1, false);
				++m_DataEnd;
			}
		}

	private:
		Block** m_RawFirst;
		Block** m_DataFirst;
		Block** m_DataEnd;
		Block** m_RawEnd;
	};

	template <bool isConst>
	class BaseIterator : public core::BaseIterator<core::BidirectionalIteratorTag, T>
	{
		friend class BaseIterator<!isConst>;
	public:
		using BlockPtr = typename core::Choose<isConst, const Block*const*, Block*const*>::type;
		BaseIterator() :
			block(nullptr),
			id(0)
		{
		}

		BaseIterator(const BaseIterator& other) :
			block(other.block),
			id(other.id)
		{
		}

		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		BaseIterator(const BaseIterator<!U>& other) :
			block(other.block),
			id(other.id)
		{
		}

		BaseIterator(BlockPtr b, int i) :
			block(b),
			id(i)
		{
		}

		BaseIterator& operator=(const BaseIterator& other)
		{
			block = other.block;
			id = other.id;
			return *this;
		}
		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		BaseIterator& operator=(const BaseIterator<!U>& other)
		{
			block = other.block;
			id = other.id;
			return *this;
		}

		BaseIterator& operator+=(intptr_t num)
		{
			auto blocks = num / BLOCK_SIZE;
			block += blocks;
			id += num % BLOCK_SIZE;
			return *this;
		}

		BaseIterator operator+ (intptr_t num) const
		{
			auto temp(*this);
			return temp += num;
		}
		BaseIterator& operator-=(intptr_t num)
		{
			return (*this) += (-num);
		}
		BaseIterator operator- (intptr_t num) const
		{
			return (*this) + (-num);
		}

		intptr_t operator-(BaseIterator other) const
		{
			intptr_t blocks = (intptr_t)(block - other.block);
			return blocks*BLOCK_SIZE + (id - other.id);
		}

		BaseIterator& operator++()
		{
			++id;
			if(id == BLOCK_SIZE) {
				id = 0;
				++block;
			}
			return *this;
		}

		BaseIterator operator++(int)
		{
			auto out(*this);
			this->operator++();
			return out;
		}

		BaseIterator& operator--()
		{
			if(id == 0) {
				id = BLOCK_SIZE;
				--block;
			}
			--id;
			return *this;
		}

		BaseIterator operator--(int)
		{
			auto out(*this);
			this->operator--();
			return out;
		}

		template <bool isConst2>
		bool operator==(const BaseIterator<isConst2>& other) const
		{
			return block == other.block && id == other.id;
		}

		template <bool isConst2>
		bool operator!=(const BaseIterator<isConst2>& other) const
		{
			return !(*this == other);
		}

		const T& operator*() const
		{
			return (*block)->data[id];
		}
		const T* operator->()
		{
			return &((*block)->data[id]);
		}

		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T& operator*()
		{
			return (*block)->data[id];
		}
		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T* operator->()
		{
			return &((*block)->data[id]);
		}

	private:
		BlockPtr block;
		int id;
	};

public:
	using Iterator = BaseIterator<false>;
	using ConstIterator = BaseIterator<true>;

	Deque() :
		m_FirstId(BLOCK_SIZE / 2),
		m_EndId(BLOCK_SIZE / 2)
	{
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

	bool operator==(const Deque& other) const
	{
		if(Size() != other.Size())
			return false;

		for(auto it = First(), jt = other.First(); it != End(); ++it, ++jt) {
			if(!(*it == *jt))
				return false;
		}
		return true;
	}

	bool operator!=(const Deque& other) const
	{
		return !(*this == other);
	}

	void Clear()
	{
		while(Size() != 0)
			PopBack();

		m_Base.Balance();
		m_FirstId = BLOCK_SIZE / 2;
		m_EndId = BLOCK_SIZE / 2;
	}

	void PushFront(const T& elem)
	{
		new (NewFrontPtr()) T(elem);
	}

	void PushFront(T&& elem)
	{
		new (NewFrontPtr()) T(std::move(elem));
	}

	template <typename... Ts>
	void EmplaceFront(Ts&&... args)
	{
		new (NewFrontPtr()) T(std::forward<Ts>(args)...);
	}

	void PushBack(const T& elem)
	{
		new (NewPackPtr()) T(elem);
	}

	void PushBack(T&& elem)
	{
		new (NewBackPtr()) T(std::move(elem));
	}

	template <typename... Ts>
	void EmplaceBack(Ts&&... args)
	{
		new (NewBackPtr()) T(std::forward<Ts>(args)...);
	}

	void PopFront()
	{
		lxAssert(Size());
		Front().~T();
		++m_FirstId;
		if(m_FirstId == BLOCK_SIZE) {
			m_Base.RemoveFront();
			m_FirstId = 0;
		}
	}

	T TakeFront()
	{
		T out(std::move(Front()));
		PopFront();
		return out;
	}

	void PopBack()
	{
		lxAssert(Size());
		Back().~T();
		--m_EndId;
		if(m_EndId == 0) {
			m_Base.RemoveBack();
			m_EndId = BLOCK_SIZE;
		}
	}

	T TakeBack()
	{
		T out(std::move(Back()));
		PopBack();
		return out;
	}

	int Size() const
	{
		return  BLOCK_SIZE*(m_Base.Size() - 1) + (m_EndId - m_FirstId);
	}

	bool IsEmpty() const
	{
		return Size() == 0;
	}

	const T& Front() const
	{
		lxAssert(Size());
		return (*m_Base.First())->data[m_FirstId];
	}

	T& Front()
	{
		lxAssert(Size());
		return (*m_Base.First())->data[m_FirstId];
	}

	const T& Back() const
	{
		lxAssert(Size());
		return (*m_Base.Last())->data[m_EndId - 1];
	}

	T& Back()
	{
		lxAssert(Size());
		return (*m_Base.Last())->data[m_EndId - 1];
	}

	ConstIterator First() const
	{
		return ConstIterator(m_Base.First(), m_FirstId);
	}

	ConstIterator End() const
	{
		if(m_EndId == BLOCK_SIZE)
			return ConstIterator(m_Base.Last() + 1, 0);
		else
			return ConstIterator(m_Base.Last(), m_EndId);
	}

	Iterator First()
	{
		return Iterator(m_Base.First(), m_FirstId);
	}

	Iterator End()
	{
		if(m_EndId == BLOCK_SIZE)
			return Iterator(m_Base.Last() + 1, 0);
		else
			return Iterator(m_Base.Last(), m_EndId);
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

template <typename T> typename Deque<T>::Iterator begin(Deque<T>& dequeu) { return dequeu.Frist(); }
template <typename T> typename Deque<T>::Iterator end(Deque<T>& dequeu) { return dequeu.End(); }
template <typename T> typename Deque<T>::ConstIterator begin(const Deque<T>& dequeu) { return dequeu.Frist(); }
template <typename T> typename Deque<T>::ConstIterator end(const Deque<T>& dequeu) { return dequeu.End(); }

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LUX_DEQUE_H