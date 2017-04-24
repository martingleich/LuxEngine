#ifndef INCLUDED_LXLIST_H
#define INCLUDED_LXLIST_H

#include "LuxBase.h"
#include "lxMemory.h"
#include "lxIterator.h"

namespace lux
{
namespace core
{

//TODO:
/*
Add push_front
Add insert
Optimize
*/

//! A double-linked list
template <typename T>
class list
{
private:
	struct ListEntry
	{
		ListEntry(const T& _data) : Next(nullptr), Prev(nullptr), data(_data)
		{
		}

		ListEntry* Next;
		ListEntry* Prev;
		T data;
	};

public:
	class ConstIterator;

	class Iterator : BaseIterator<BidirectionalIteratorTag, T>
	{
	public:
		Iterator() : m_Current(nullptr)
		{
		}

		Iterator& operator++()
		{
			m_Current = m_Current->Next;
			return *this;
		}
		Iterator& operator--()
		{
			m_Current = m_Current->Prev;
			return *this;
		}
		Iterator  operator++(int)
		{
			Iterator tmp(*this);
			m_Current = m_Current->Next;
			return tmp;
		}
		Iterator  operator--(int)
		{
			Iterator tmp(*this);
			m_Current = m_Current->Prev;
			return tmp;
		}

		Iterator& operator+=(int num)
		{
			if(num > 0)
				while(num-- && this->m_Current != 0) ++(*this);
			else
				while(num++ && this->m_Current != 0) --(*this);

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

		bool operator==(const Iterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const Iterator& other) const
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

		T& operator*()
		{
			return m_Current->data;
		}
		T* operator->()
		{
			return &(m_Current->data);
		}

	private:
		explicit Iterator(ListEntry* pBegin) : m_Current(pBegin)
		{
		}
		friend class list<T>;
		friend class ConstIterator;

	private:
		ListEntry* m_Current;
	};

	class ConstIterator : BaseIterator<BidirectionalIteratorTag, T>
	{
	public:
		ConstIterator() : m_Current(nullptr)
		{
		}
		ConstIterator(const Iterator& iter) : m_Current(iter.m_Current)
		{
		}

		ConstIterator& operator++()
		{
			m_Current = m_Current->Next; return *this;
		}
		ConstIterator& operator--()
		{
			m_Current = m_Current->Prev; return *this;
		}
		ConstIterator  operator++(int)
		{
			ConstIterator tmp(*this);
			m_Current = m_Current->Next;
			return tmp;
		}
		ConstIterator  operator--(int)
		{
			ConstIterator tmp(*this);
			m_Current = m_Current->Prev;
			return tmp;
		}

		ConstIterator& operator+=(int num)
		{
			if(num > 0)
				while(num-- && this->m_Current != nullptr) ++(*this);
			else
				while(num++ && this->m_Current != nullptr) --(*this);

			return *this;
		}

		ConstIterator  operator+ (int num) const
		{
			ConstIterator temp = *this; return temp += num;
		}
		ConstIterator& operator-=(int num)
		{
			return (*this) += (-num);
		}
		ConstIterator  operator- (int num) const
		{
			return (*this) + (-num);
		}

		bool operator==(const Iterator& other) const
		{
			return m_Current == other.m_Current;
		}
		bool operator!=(const Iterator& other) const
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

		const T& operator*()
		{
			return m_Current->data;
		}
		const T* operator->()
		{
			return &(m_Current->data);
		}

		ConstIterator& operator=(const Iterator& iter)
		{
			m_Current = iter.m_Current; return *this;
		}

	private:
		explicit ConstIterator(ListEntry* pBegin) : m_Current(pBegin)
		{
		}

		friend class Iterator;
		friend class list<T>;
	private:
		ListEntry* m_Current;
	};

	//! default constructor
	/**
	Create an empty list
	*/
	list() : m_First(nullptr), m_Last(nullptr), m_Size(0)
	{
	}

	//! Copyconstructor
	list(const list<T>& other) : m_First(nullptr), m_Last(nullptr), m_Size(0)
	{
		*this = other;
	}

	//! Move constructor
	list(list<T>&& old) : m_First(old.m_Frist), m_Last(old.m_Last), m_Size(old.m_Size)
	{
		old.m_First = nullptr;
		old.m_Last = nullptr;
		old.m_Size = 0;
	}

	//! Destructor
	~list()
	{
		Clear();
	}

	//! Assignment
	list<T>& operator=(const list<T>& other)
	{
		if(&other == this)
			return *this;

		Clear();

		ListEntry* entry = other.m_First;
		while(entry) {
			Push_back(entry->data);
			entry = entry->Next;
		}

		return *this;
	}

	//! Move assignment
	list<T>& operator=(list<T>&& old)
	{
		Clear();
		m_First = old.m_First;
		m_Last = old.m_Last;
		m_Size = old.m_Size;
		old.m_First = nullptr;
		old.m_Last = nullptr;
		old.m_Size = 0;
	}

	//! The size of the list
	/**
	\return The number of elements in the list
	*/
	u32 Size() const
	{
		return m_Size;
	}

	//! Remove all elements from the list
	void Clear()
	{
		while(m_First) {
			ListEntry* Next = m_First->Next;
			delete m_First;
			m_First = Next;
		}

		m_First = nullptr;
		m_Last = nullptr;
		m_Size = 0;
	}

	//! Is the list empty
	/**
	\return True if no element are in the list, otherwise false
	*/
	bool Empty() const
	{
		return (m_First == nullptr);
	}

	//! Add an element to the end of the list
	/**
	\param element The element to add to the list
	*/
	void Push_back(const T& element)
	{
		ListEntry* entry = new ListEntry(element);

		++m_Size;

		// Ist es der erste Eintrag
		if(m_First == nullptr)
			m_First = entry;

		entry->Prev = m_Last;

		if(m_Last != nullptr)
			m_Last->Next = entry;

		m_Last = entry;
	}


	//! Iterator to the first element in the list
	/**
	\return The first element in the list
	*/
	Iterator First()
	{
		return Iterator(m_First);
	}

	//! ConstIterator to the first element in the list
	/**
	\return The first element in the list
	*/
	ConstIterator First() const
	{
		return ConstIterator(m_First);
	}

	//! Iterator to the element after the last in the list
	/**
	\return The element after the last
	*/
	Iterator End()
	{
		return Iterator(nullptr);
	}

	//! ConstIterator to the element after the last in the list
	/**
	\return The element after the last
	*/
	ConstIterator End() const
	{
		return ConstIterator(nullptr);
	}

	//! Iterator to the last element in the list
	/**
	\return The last element in the list
	*/
	Iterator Last()
	{
		return Iterator(m_Last);
	}

	//! ConstIterator to the last element in the list
	/**
	\return The last element in the list
	*/
	ConstIterator Last() const
	{
		return ConstIterator(m_Last);
	}

	//! Iterator to the element before the first in the list
	/**
	\return The element before the first
	*/
	Iterator Begin()
	{
		return Iterator(nullptr);
	}

	//! ConstIterator to the element before the first in the list
	/**
	\return The element before the first
	*/
	ConstIterator Begin() const
	{
		return ConstIterator(nullptr);
	}

	//! Erase an element from the list
	/**
	\param it The element to remove
	\return An iterator to the next element in the list
	*/
	Iterator Erase(Iterator it)
	{
		Iterator ReturnIter(it);
		++ReturnIter;

		if(it.m_Current == m_First) {
			m_First = it.m_Current->Next;
		} else {
			it.m_Current->Prev->Next = it.m_Current->Next;
		}

		if(it.m_Current == m_Last) {
			m_Last = it.m_Current->Prev;
		} else {
			it.m_Current->Next->Prev = it.m_Current->Prev;
		}

		delete it.m_Current;
		it.m_Current = nullptr;
		--m_Size;

		return ReturnIter;
	}

private:
	u32           m_Size;
	ListEntry*    m_First;
	ListEntry*    m_Last;
};


} 

} 


#endif
