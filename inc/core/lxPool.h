#ifndef INCLUDED_LUX_POOL_H
#define INCLUDED_LUX_POOL_H
#include "LuxBase.h"
#include "lxIterator.h"
#include "lxMemory.h"

namespace lux
{
namespace core
{

//! Pool container
/**
A pool is optimized for repeated creation and destruction
of unordered data
*/
template <typename T>
class Pool
{
public:
	class ConstIterator;

	class Iterator : public core::BaseIterator<RandomAccessIteratorTag, T>
	{
	public:
		Iterator() : m_Current(nullptr)
		{
		}

		Iterator& operator++()
		{
			++m_Current; return *this;
		}
		Iterator& operator--()
		{
			--m_Current; return *this;
		}
		Iterator  operator++(int)
		{
			Iterator Temp = *this; ++m_Current; return Temp;
		}
		Iterator  operator--(int)
		{
			Iterator Temp = *this; --m_Current; return Temp;
		}

		Iterator& operator+=(int num)
		{
			m_Current += num;

			return *this;
		}

		Iterator  operator+ (int num) const
		{
			Iterator temp = *this; return temp += num;
		}
		Iterator& operator-=(int num)
		{
			return (*this) += (-num);
		}
		Iterator  operator- (int num) const
		{
			return (*this) + (-num);
		}

		bool operator==(const Iterator&         other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const Iterator&         other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const ConstIterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ConstIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		T& operator*() const { return *m_Current; }
		T* operator->() const { return m_Current; }

	private:
		explicit Iterator(T* _current) : m_Current(_current)
		{
		}
		friend class Pool<T>;
		friend class ConstIterator;

	private:
		T* m_Current;
	};

	class ConstIterator : public core::BaseIterator<RandomAccessIteratorTag, T>
	{
	public:
		ConstIterator() : m_Current(nullptr)
		{
		}
		ConstIterator(const ConstIterator& iter) : m_Current(iter.m_Current)
		{
		}
		ConstIterator(const Iterator& iter) : m_Current(iter.m_Current)
		{
		}

		ConstIterator& operator++()
		{
			++m_Current; return *this;
		}
		ConstIterator& operator--()
		{
			--m_Current; return *this;
		}
		ConstIterator  operator++(int)
		{
			Iterator Temp = *this; ++m_Current; return Temp;
		}
		ConstIterator  operator--(int)
		{
			Iterator Temp = *this; --m_Current; return Temp;
		}

		ConstIterator& operator+=(int num)
		{
			m_Current += num;

			return *this;
		}

		ConstIterator  operator+ (int num) const
		{
			Iterator temp = *this; return temp += num;
		}
		ConstIterator& operator-=(int num)
		{
			return (*this) += (-num);
		}
		ConstIterator  operator- (int num) const
		{
			return (*this) + (-num);
		}

		bool operator==(const Iterator&         other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const Iterator&         other) const
		{
			return m_Current != other.m_Current;
		}
		bool operator==(const ConstIterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const ConstIterator& other) const
		{
			return m_Current != other.m_Current;
		}

		const T& operator*() const { return *m_Current; }
		const T* operator->() const { return m_Current; }

		ConstIterator& operator=(const Iterator& iter)
		{
			m_Current = iter.m_Current; return *this;
		}
		ConstIterator& operator=(const ConstIterator& iter)
		{
			m_Current = iter.m_Current; return *this;
		}
	private:
		explicit ConstIterator(const T* _current) : m_Current(_current)
		{
		}

		friend class Iterator;
		friend class Pool<T>;

	private:
		const T* m_Current;
	};

public:
	//! Create a pool with a start capazity
	/**
	\param capactatiy The max number of elements in the pool
	*/
	Pool(int capacity = 1024)
	{
		lxAssert(capacity > 0);

		m_Data = LUX_NEW_ARRAY(T, capacity);
		m_Alloc = capacity;
		m_Active = 0;
		m_AutoAllocation = false;
	}

	Pool(const Pool<T>& other)
	{
		*this = other;
	}

	Pool(Pool<T>&& old)
	{
		*this = std::move(old);
	}

	Pool& operator=(const Pool<T>& other)
	{
		m_Alloc = other.m_Alloc;
		m_Active = other.m_Active;
		m_AutoAllocation = other.m_AutoAllocation;
		m_Data = LUX_NEW_ARRAY(T, m_Alloc);
		for(int i = 0; i < m_Active; ++i)
			m_Data[i] = std::move(other.m_Data);
	}

	Pool& operator=(Pool<T>&& old)
	{
		m_Alloc = old.m_Alloc;
		m_Active = old.m_Active;
		m_AutoAllocation = old.m_AutoAllocation;
		m_Data = old.m_Data;
		old.m_Data = nullptr;
	}

	//! The number of active elements
	/**
	\return The number of active elements
	*/
	int GetActiveCount() const
	{
		return m_Active;
	}

	//! The number of inactive elements
	/**
	\return The number of inactive elements
	*/
	int GetInactiveCount() const
	{
		return m_Alloc - m_Active;
	}

	//! The maximal size of the pool
	/**
	\return The maximal size of the pool
	*/
	int Capactity() const
	{
		return m_Alloc;
	}
	//! Iterator to the first active element
	/**
	\return The first active element
	*/
	Iterator First()
	{
		return Iterator(m_Data);
	}

	//! Iterator behind the last active element
	/**
	\return The element after the last active element
	*/
	Iterator End()
	{
		return Iterator(m_Data + m_Active);
	}

	//! ConstIterator to the first active element
	/**
	\return The first active element
	*/
	ConstIterator First() const
	{
		return ConstIterator(m_Data);
	}

	//! ConstIterator behind the last active element
	/**
	\return The element after the last active element
	*/
	ConstIterator End() const
	{
		return ConstIterator(m_Data + m_Active);
	}

	//! Iterator before the first active element
	/**
	\return The element before the first active element
	*/
	Iterator Begin()
	{
		return Iterator(m_Data - 1);
	}

	//! Iterator to the last active element
	/**
	\return The last active element
	*/
	Iterator Last()
	{
		return Iterator(m_Data + m_Active - 1);
	}

	//! ConstIterator before the first active element
	/**
	\return The element before the first active element
	*/
	ConstIterator Begin() const
	{
		return ConstIterator(m_Data - 1);
	}

	//! ConstIterator to the last active element
	/**
	\return The last active element
	*/
	ConstIterator Last() const
	{
		return ConstIterator(m_Data + m_Active - 1);
	}

	//! Add a new active element to the pool
	/*
	The order of the element is not changed by this action
	\param elem The new active element
	\return Was the element added to the pool
	*/
	bool PushActive(const T& elem)
	{
		if(GetInactiveCount() > 0) {
			m_Data[m_Active] = elem;
			m_Active++;
			return true;
		}

		if(m_AutoAllocation) {
			Reserve((3 * m_Alloc) / 2);
			return PushActive(elem);
		}

		return false;
	}


	//! Add a new active element to the pool
	/*
	The order of the element is not changed by this action
	\param elem The new active element
	\return Was the element added to the pool
	*/
	bool PushActive(T&& elem)
	{
		if(GetInactiveCount() > 0) {
			m_Data[m_Active] = elem;
			m_Active++;
			return true;
		}

		if(m_AutoAllocation) {
			Reserve((3 * m_Alloc) / 2);
			return PushActive(elem);
		}

		return false;
	}


	//! Create a new active element in the pool
	/*
	The order of the element is not changed by this action
	\return The newly created element in the pool, or NULL if no element is avaiable
		This element contains the last removed element,
		or a newly constructed element
	*/
	T* MakeActive()
	{
		if(GetInactiveCount() > 0)
			return &m_Data[m_Active++];

		if(m_AutoAllocation) {
			Reserve((3 * m_Alloc) / 2);
			return &m_Data[m_Active++];
		}

		return nullptr;
	}

	//! Enable auto allocation
	/**
	With enabled auto allocation, new memory is added to the pool
	if there is no more space for new element
	Without the add operation fails
	\param Auto Should autoallocation be used
	\ref IsAutoAllocation
	*/
	void SetAutoAllocation(bool Auto)
	{
		m_AutoAllocation = Auto;
	}

	//! Is auto allocation enables
	/**
	With enabled auto allocation, new memory is added to the pool
	if there is no more space for new element
	Without the add operation fails
	\param Auto Should autoallocation be used
	\ref SetAutoAllocation
	*/
	bool IsAutoAllocation() const
	{
		return m_AutoAllocation;
	}

	//! Disables a single element in the pool
	/**
	This operation changes the order of elements after the iterator
	\param iter The element to disable
	*/
	void Disable(Iterator iter)
	{
		T Tmp = std::move(*iter);
		*iter = std::move(*Last());
		*Last() = Tmp;

		--m_Active;
	}

	void Disable(T* x)
	{
		lxAssert(x >= m_Data && x <= m_Data+m_Active);

		Disable(Iterator(x));
	}

	//! Disables all elements in the pool
	void DisableAll()
	{
		m_Active = 0;
	}

	//! Allocated new element to the pool
	/**
	\param capacity The number of element to add to the pool
	*/
	void Reserve(int capacity)
	{
		m_Alloc += capacity;
		T* newData = LUX_NEW_ARRAY(T, m_Alloc);
		for(int i = 0; i < m_Active; ++i)
			newData[i] = std::move(m_Data[i]);
		LUX_FREE_ARRAY(m_Data);

		m_Data = newData;
	}

private:
	T*  m_Data;
	int m_Alloc;
	int m_Active;

	bool m_AutoAllocation;
};

template <typename T> inline typename Pool<T>::Iterator begin(Pool<T>& pool) { return pool.First(); }
template <typename T> inline typename Pool<T>::Iterator end(Pool<T>& pool) { return pool.End(); }
template <typename T> inline typename Pool<T>::ConstIterator begin(const Pool<T>& pool) { return pool.First(); }
template <typename T> inline typename Pool<T>::ConstIterator end(const Pool<T>& pool) { return pool.End(); }

} // !namespace core
} // !namespace lux

#endif // !INCLUDED_LUX_POOL_H
