#ifndef INCLUDED_LUX_LIST_H
#define INCLUDED_LUX_LIST_H
#include "LuxBase.h"
#include "lxMemory.h"
#include "lxIterator.h"

namespace lux
{
namespace core
{

//! A double-linked list
template <typename T>
class List
{
private:
	struct Entry
	{
		Entry* next;
		Entry* prev;
		T data;
	};

	template <bool isConst>
	class BaseIterator : public core::BaseIterator<core::BidirectionalIteratorTag, T>
	{
		friend class BaseIterator<!isConst>;
		friend class List;

	public:
		using EntryPtr = typename core::Choose<isConst, const Entry*, Entry*>::type;
		BaseIterator() :
			m_Current(nullptr)
		{
		}
		explicit BaseIterator(EntryPtr cur) :
			m_Current(cur)
		{
		}

		BaseIterator(const BaseIterator& other) :
			m_Current(other.m_Current)
		{
		}

		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		BaseIterator(const BaseIterator<!U>& other) :
			m_Current(other.m_Current)
		{
		}

		BaseIterator& operator=(const BaseIterator& other)
		{
			m_Current = other.m_Current;
			return *this;
		}

		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		BaseIterator& operator=(const BaseIterator<!U>& other)
		{
			m_Current = other.m_Current;
			return *this;
		}

		BaseIterator& operator++()
		{
			m_Current = m_Current->next;
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
			m_Current = m_Current->prev;
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
			return m_Current == other.m_Current;
		}

		template <bool isConst2>
		bool operator!=(const BaseIterator<isConst2>& other) const
		{
			return !(*this == other);
		}

		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		const T& operator*() const { return m_Current->data; }
		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		const T* operator->() const { return &m_Current->data; }

		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T& operator*() const { return m_Current->data; }
		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T* operator->() const { return &m_Current->data; }

	private:
		EntryPtr m_Current;
	};

public:
	using Iterator = BaseIterator<false>;
	using ConstIterator = BaseIterator<true>;

	//! default constructor
	/**
	Create an empty list
	*/
	List() :
		m_First(nullptr),
		m_Last(nullptr),
		m_Size(0)
	{
	}

	//! Copyconstructor
	List(const List& other) :
		m_First(nullptr),
		m_Last(nullptr),
		m_Size(0)
	{
		*this = other;
	}

	//! Move constructor
	List(List&& old) :
		m_First(old.m_Frist),
		m_Last(old.m_Last),
		m_Size(old.m_Size)
	{
		old.m_First = nullptr;
		old.m_Last = nullptr;
		old.m_Size = 0;
	}

	//! Destructor
	~List()
	{
		Clear();
	}

	//! Assignment
	List& operator=(const List& other)
	{
		if(&other == this)
			return *this;

		Clear();

		for(Entry* it = other.m_First; it; it = it->next)
			PushBack(it->data);

		return *this;
	}

	//! Move assignment
	List& operator=(List&& old)
	{
		Clear();
		m_First = old.m_First;
		m_Last = old.m_Last;
		m_Size = old.m_Size;
		old.m_First = nullptr;
		old.m_Last = nullptr;
		old.m_Size = 0;
		return *this;
	}

	//! Remove all elements from the list
	void Clear()
	{
		while(m_First) {
			Entry* next = m_First->next;
			m_First->data.~T();
			LUX_FREE(m_First);
			m_First = next;
		}

		m_First = nullptr;
		m_Last = nullptr;
		m_Size = 0;
	}

	//! Add an element at the end of the list.
	void PushBack(const T& element)
	{
		new (GetInsertPointer(End())) T(element);
	}

	//! Add an element at the end of the list.
	void PushBack(T&& element)
	{
		new (GetInsertPointer(End())) T(std::move(element));
	}

	//! Add an element at the end of the list.
	template <typename... Ar>
	void EmplaceBack(Ar&&... args)
	{
		new (GetInsertPointer(End())) T(std::forward<Ar>(args)...);
	}

	//! Add an element at the beginning of the list.
	void PushFront(const T& element)
	{
		new (GetInsertPointer(First())) T(element);
	}

	//! Add an element at the beginning of the list.
	void PushFront(T&& element)
	{
		new (GetInsertPointer(First())) T(std::move(element));
	}

	//! Add an element at the beginning of the list.
	template <typename... Ar>
	void EmplaceFront(Ar&&... args)
	{
		new (GetInsertPointer(First())) T(std::forward<Ar>(args)...);
	}

	//! Add an element to the list.
	/**
	\param pos Position where the new element is placed.
	\param element The element to insert.
	*/
	void Insert(Iterator pos, const T& element)
	{
		new (GetInsertPointer(pos)) T(element);
	}

	//! Add an element to the list.
	/**
	\param pos Position where the new element is placed.
	\param element The element to insert.
	*/
	void Insert(Iterator pos, T&& element)
	{
		new (GetInsertPointer(pos)) T(std::move(element));
	}

	//! Add an element to the list.
	/**
	\param pos Position where the new element is placed.
	\param element The element to insert.
	*/
	template <typename... Ar>
	void Emplace(Iterator pos, Ar&&... args)
	{
		new (GetInsertPointer(pos)) T(std::forward(args)...);
	}

	//! Remove the first element from the list.
	void PopFront()
	{
		Erase(First());
	}

	//! Remove the last element from the list.
	void PopBack()
	{
		Erase(Last());
	}

	//! Remove an element from the list.
	/**
	\param it The element to remove.
	\return The next element after the deleted one.
	*/
	Iterator Erase(Iterator it)
	{
		lxAssert(Size() > 0);

		Iterator out(it);
		++out;

		auto ptr = it.m_Current;
		if(ptr == m_First)
			m_First = ptr->next;
		else
			ptr->prev->next = ptr->next;

		if(ptr == m_Last)
			m_Last = ptr->prev;
		else
			ptr->next->prev = ptr->prev;

		--m_Size;

		ptr->data.~T();
		LUX_FREE(ptr);

		return out;
	}

	//! The size of the list
	/**
	\return The number of elements in the list
	*/
	int Size() const
	{
		return m_Size;
	}

	//! Is the list empty
	/**
	\return True if no element are in the list, otherwise false
	*/
	bool IsEmpty() const
	{
		return (Size() == 0);
	}

	Iterator First() { return Iterator(m_First); }
	ConstIterator First() const { return ConstIterator(m_First); }
	Iterator End() { return Iterator(nullptr); }
	ConstIterator End() const { return ConstIterator(nullptr); }
	Iterator Last() { return Iterator(m_Last); }
	ConstIterator Last() const { return ConstIterator(m_Last); }
	Iterator Begin() { return Iterator(nullptr); }
	ConstIterator Begin() const { return ConstIterator(nullptr); }

	//! The first element in the list.
	T& Front() { return m_First->data; }
	//! The first element in the list.
	const T& Front() const { return m_First->data; }

	//! The last element in the list.
	T& Back() { return m_Last->data; }
	//! The last element in the list.
	const T& Back() const { return m_Last->data; }

private:
	T* GetInsertPointer(Iterator pos)
	{
		Entry* entry = LUX_NEW(Entry);
		auto ptr = pos.m_Current;
		if(ptr == nullptr) { // Push back
			entry->next = nullptr;
			entry->prev = m_Last;
			if(m_Last)
				m_Last->next = entry;
			m_Last = entry;
		} else {
			entry->prev = ptr->prev;
			entry->next = ptr;
			if(ptr->prev)
				ptr->prev->next = entry;
			ptr->prev = entry;
		}

		if(!m_First || !entry->prev)
			m_First = entry;
		if(!m_Last || !entry->next)
			m_Last = entry;

		++m_Size;
		return &entry->data;
	}

private:
	Entry* m_First;
	Entry* m_Last;
	int m_Size;
};

template <typename T> typename List<T>::Iterator begin(List<T>& list) { return list.Frist(); }
template <typename T> typename List<T>::Iterator end(List<T>& list) { return list.End(); }
template <typename T> typename List<T>::ConstIterator begin(const List<T>& list) { return list.Frist(); }
template <typename T> typename List<T>::ConstIterator end(const List<T>& list) { return list.End(); }

} // namespace core
} // namespace lux

#endif