#ifndef INCLUDED_LXARRAY_H
#define INCLUDED_LXARRAY_H
#include "math/lxMath.h"
#include "core/lxSort.h"
#include "core/lxMemory.h"
#include "core/lxIterator.h"
#include "core/lxTypes.h"
#include <initializer_list>
#include <type_traits>

namespace lux
{
namespace core
{

//! A template dynamic array
template <typename T>
class Array
{
private:
	template <bool isConst>
	class ArrayIterator : public BaseIterator<RandomAccessIteratorTag, T>
	{
		friend class Array<T>;
		using PtrT = typename core::Choose<isConst, const T*, T*>::type;
		static const bool IS_CONST = isConst;
	public:
		ArrayIterator() : m_Current(nullptr)
		{
		}
		explicit ArrayIterator(PtrT ptr) : m_Current(ptr)
		{
		}
		ArrayIterator(const ArrayIterator<isConst>& iter) :
			m_Current(iter.m_Current)
		{
		}
		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		ArrayIterator(const ArrayIterator<!U>& iter) :
			m_Current(iter.m_Current)
		{
		}

		template <bool U = isConst, std::enable_if_t<U, int> = 0>
		ArrayIterator& operator=(const ArrayIterator<!U>& iter)
		{
			m_Current = iter.m_Current;
			return *this;
		}
		ArrayIterator& operator=(const ArrayIterator<isConst>& iter)
		{
			m_Current = iter.m_Current;
			return *this;
		}

		ArrayIterator& operator++() { ++m_Current; return *this; }
		ArrayIterator& operator--() { --m_Current; return *this; }
		ArrayIterator operator++(int)
		{
			ArrayIterator tmp(*this); ++m_Current; return tmp;
		}
		ArrayIterator operator--(int)
		{
			ArrayIterator tmp = *this; --m_Current; return tmp;
		}

		ArrayIterator& operator+=(intptr_t num)
		{
			m_Current += num;
			return *this;
		}

		ArrayIterator operator+(intptr_t num) const
		{
			ArrayIterator temp = *this; return temp += num;
		}
		ArrayIterator& operator-=(intptr_t num)
		{
			return (*this) += (-num);
		}
		ArrayIterator operator-(intptr_t num) const
		{
			return (*this) + (-num);
		}

		intptr_t operator-(ArrayIterator other) const
		{
			return m_Current - other.m_Current;
		}

		template <bool isConst2>
		bool operator==(const ArrayIterator<isConst2>& other) const
		{
			return m_Current == other.m_Current;
		}
		template <bool isConst2>
		bool operator!=(const ArrayIterator<isConst2>& other) const
		{
			return m_Current != other.m_Current;
		}

		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T& operator*()
		{
			return *m_Current;
		}

		const T& operator*() const
		{
			return *m_Current;
		}

		template <bool U = !isConst, std::enable_if_t<U, int> = 0>
		T* operator->()
		{
			return m_Current;
		}

		const T* operator->() const
		{
			return m_Current;
		}
	private:
		PtrT m_Current;
	};

public:
	using Iterator = ArrayIterator<false>;
	using ConstIterator = ArrayIterator<true>;

public:
	//! Constructor
	/**
	Create an empty array
	*/
	Array()
	{
	}

	Array(std::initializer_list<T> init)
	{
		Reserve(init.size());
		for(auto& e : init)
			PushBack(std::move(e));
	}

	//! Destruktor
	~Array()
	{
		Clear();
	}

	//! Move constructor
	Array(Array<T>&& old)
	{
		m_Data = old.m_Data;
		m_Used = old.m_Used;
		m_Alloc = old.m_Alloc;
		old.m_Data = nullptr;
	}

	//! Move assignment 
	Array<T>& operator=(Array<T>&& old)
	{
		lxAssert(this != &old);

		if(Data()) {
			for(size_t i = 0; i < m_Used; ++i)
				(Data() + i)->~T();
			ArrayFree(Data());
		}

		m_Data = old.m_Data;
		m_Used = old.m_Used;
		m_Alloc = old.m_Alloc;
		old.m_Data = nullptr;
		old.m_Used = 0;
		old.m_Alloc = 0;

		return *this;
	}

	//! Copyconstructor
	Array(const Array<T>& other)
	{
		*this = other;
	}

	//! Assignment
	Array<T>& operator=(const Array<T>& other)
	{
		// Alte Einträge löschen
		if(m_Data)
			Clear();

		// Neu reservieren
		if(other.m_Alloc == 0)
			m_Data = nullptr;
		else
			m_Data = (T*)ArrayAllocate(other.m_Alloc * sizeof(T));

		// Alles kopieren
		m_Used = other.m_Used;
		m_Alloc = other.m_Alloc;

		lxAssert(m_Used <= m_Alloc);

		if(m_Data) {
			for(size_t i = 0; i < m_Used; ++i)
				new (Data() + i) T(other.Data()[i]);
		}

		return *this;
	}

	//! Comparison
	bool operator==(const Array<T>& other) const
	{
		if(Size() != other.Size())
			return false;
		for(size_t i = 0; i < other.Size(); ++i) {
			if(Data_c()[i] != other.Data_c()[i])
				return false;
		}
		return true;
	}
	bool operator!=(const Array<T>& other) const
	{
		return !(*this == other);
	}

	//! Add a new entry at any position to the array
	/**
	\param entry The new entry
	\param before entry is build at this position
	*/
	void Insert(const T& entry, Iterator before)
	{
		auto ptr = GetInsertPointer(before);
		if(ptr == Data() + Size() - 1)
			new (ptr) T(entry);
		else
			*ptr = entry;
	}

	//! Add a new entry at any position to the array(move version)
	/**
	\param entry The new entry
	\param before entry is build at this position
	*/
	void Insert(T&& entry, Iterator before)
	{
		auto ptr = GetInsertPointer(before);
		if(ptr == Data() + m_Used - 1)
			new (ptr) T(std::move(entry));
		else
			*ptr = std::move(entry);
	}

	template <typename... Ts>
	T& Emplace(Iterator before, Ts&&... args)
	{
		auto ptr = GetInsertPointer(before, true);
		new (ptr) T(std::forward<Ts>(args)...);
		return *ptr;
	}

	//! Add a new entry to the end of the array
	/**
	\param entry The entry to add
	*/
	void PushBack(const T& entry)
	{
		Insert(entry, End());
	}

	//! Add a new entry to the end of the array(move version)
	/**
	\param entry The entry to add
	*/
	void PushBack(T&& entry)
	{
		Insert((T&&)entry, End());
	}

	template <typename... Ts>
	T& EmplaceBack(Ts&&... args)
	{
		auto ptr = GetInsertPointer(End());
		new (ptr) T(std::forward<Ts>(args)...);
		return *ptr;
	}

	//! Insert a new entry a the beginning of the array
	/**
	Very slow operation, the whole array have to be moved
	\param entry The entry to insert
	*/
	void PushFront(const T& entry)
	{
		Insert(entry, First());
	}

	//! Insert a new entry a the beginning of the array(move version)
	/**
	Very slow operation, the whole array have to be moved
	\param entry The entry to insert
	*/
	void PushFront(T&& entry)
	{
		Insert(entry, First());
	}

	template <typename... Ts>
	T& EmplaceFront(Ts&&... args)
	{
		auto ptr = GetInsertPointer(First(), true);
		new (ptr) T(std::forward<Ts>(args)...);
		return *ptr;
	}

	void PushBack(const core::Array<T>& entries)
	{
		PushBack(entries.Data_c(), entries.Size());
	}

	//! Add multiple entries to the end of the array
	/**
	More efficent than multiple call to PushBack
	\param entries A pointer to then entries to add
	\param numEntries The number of entries to add
	*/
	void PushBack(const T* entries, size_t numEntries)
	{
		if(m_Used + numEntries > m_Alloc)
			Reserve(m_Alloc + numEntries);

		for(size_t entry = 0; entry < numEntries; ++entry)
			new ((void*)(Data() + m_Used + entry))T(entries[entry]);

		m_Used += numEntries;
	}

	//! Add multiple entries to the end of the array(move version)
	/**
	More efficent than multiple call to PushBack
	\param entries A pointer to then entries to add
	\param numEntries The number of entries to add
	*/
	void PushBack(T* entries, size_t numEntries)
	{
		// Wenn kein Platz mehr ist welchen machen
		if(m_Used + numEntries > m_Alloc) {
			// Mit den Plätzen auf Nummer sicher gehen
			Reserve(m_Alloc + numEntries);
		}

		for(size_t entry = 0; entry < numEntries; ++entry) {
			// Listeneintrag erzeugen
			new ((void*)(Data() + m_Used + entry))T(std::move(entries[entry]));
		}

		// Vieleicht soll das Feld, nie sortiert werden und definiert keinen Vergleichsoperator,
		// dann müsste jeder Typ den Vergleichsoperator implementieren
		m_Used += numEntries;
	}

	//! Remove the last element in the array
	void PopBack()
	{
		Erase(Last(), true);
	}

	//! Remove the last element in the array
	void PopFront()
	{
		Erase(First(), true);
	}

	//! Sort the array if it wasnt already sorted
	/**
	Uses the operators = and < to sort the array
	*/
	void Sort()
	{
		if(m_Used > 1)
			core::Sort(*this, core::CompareType<T>());
	}

	template <typename CompareT>
	void Sort(const CompareT& compare)
	{
		if(m_Used > 1)
			core::Sort(*this, compare);
	}

	//! Remove an entry in the array
	/**
	\param it The element to remove
	\param HoldOrder Should the elements in array be in the same order as before
	\return The iterator of the element that took the place of the erased, if
		the last element of the array was erased, returns the end iterator.
	*/
	Iterator Erase(Iterator it, bool holdOrder = false)
	{
		size_t cur = it.m_Current - Data();
		lxAssert(it.m_Current - Data() <= (int)m_Used);

		if(holdOrder) {
			for(T* i = it.m_Current; i < Data() + m_Used - 1; ++i)
				*(i) = std::move(*(i + 1));
		} else {
			if(m_Used > 1)
				*it = std::move(*Last());
		}

		Data()[m_Used - 1].~T();
		m_Used--;
		if(m_Used * 2 <= m_Alloc)
			Optimize();

		return Iterator(Data() + cur);
	}

	//! Remove multiple elements from the array
	/**
	\param from The first element to remove.
	\param count The number of elements to remove.
	\param HoldOrder Should the elements in array be in the same order as before
	\return The iterator of the element that took the place of the first erased, if
		the last element of the array was erased, returns the end iterator.
	*/
	Iterator Erase(Iterator from, size_t count, bool holdOrder)
	{
		size_t cur = from.m_Current - Data();
		lxAssert((from.m_Current - Data()) + count - 1 <= m_Used);

		if(holdOrder) {
			for(T* i = from.m_Current; i < Data() + m_Used - count; ++i)
				*(i) = std::move(*(i + count));
		} else {
			size_t offset = m_Used - count - (from.m_Current - Data());
			size_t ToCopy = math::Min(count, offset);

			for(size_t i = 0; i < ToCopy; ++i)
				*(from.m_Current + i) = std::move(*(from.m_Current + offset + i));
		}

		for(size_t i = 0; i < count; ++i) {
			Data()[m_Used - 1 - i].~T();
		}

		m_Used -= count;

		if(m_Used * 2 <= m_Alloc)
			Optimize();

		return Iterator(Data() + cur);
	}


	//! Remove all elements from the list
	/**
	Elements arent destructed one for one
	*/
	void Clear()
	{
		if(Data()) {
			for(size_t i = 0; i < m_Used; ++i)
				Data()[i].~T();
			ArrayFree(Data());

			m_Data = nullptr;
			m_Used = 0;
			m_Alloc = 0;
		}
	}

	//! Release unused memory
	void Optimize()
	{
		Reserve(m_Used);
	}

	//! Set the reserved size of the list
	/**
	\param newSize The new reserved size of the list
	*/
	void Reserve(size_t newSize)
	{
		T* newEntries = (T*)ArrayAllocate(newSize * sizeof(T));

		if(m_Data) {
			size_t end = m_Used < (size_t)(newSize) ? m_Used : (size_t)(newSize);
			for(size_t i = 0; i < end; ++i) {
				new ((void*)&newEntries[i])T(std::move(Data()[i]));
				(Data() + i)->~T();
			}
			ArrayFree(Data());
		}
		m_Data = newEntries;

		m_Alloc = newSize;
	}

	//! Set the number of used elements aka the size of the array
	/**
	Only used elements can be accessed via [] operator
	\param used The new size of the list
	\param defaultValue The value of new elements.
	*/
	void Resize(size_t used, const T& defaultValue)
	{
		if(m_Alloc < used) {
			Reserve(used);
		} else {
			for(size_t i = used; i < m_Used; ++i)
				Data()[i].~T();
		}

		for(size_t i = m_Used; i < used; ++i)
			new ((void*)&Data()[i]) T(defaultValue);

		m_Used = used;
	}

	//! Set the number of used elements aka the size of the array
	/**
	Only used elements can be accessed via [] operator.
	New elements are default constructed
	\param used The new size of the list
	*/
	void Resize(size_t used)
	{
		if(m_Alloc < used) {
			Reserve(used);
			for(size_t i = m_Used; i < used; ++i)
				new ((void*)&Data()[i]) T();
		} else {
			for(size_t i = used; i < m_Used; ++i)
				Data()[i].~T();
		}

		for(size_t i = m_Used; i < used; ++i)
			new ((void*)&Data()[i]) T();

		m_Used = used;
	}
	//! Iterator to the first element in the array
	/**
	\return The first element in the array
	*/
	Iterator First()
	{
		return Iterator(Data());
	}

	//! ConstIterator to the first element in the array
	/**
	\return The first element in the array
	*/
	ConstIterator First() const
	{
		return FirstC();
	}

	ConstIterator FirstC() const
	{
		return ConstIterator(Data());
	}
	//! Iterator to the last element in the array
	/**
	\return The element beyond the last in the array
	*/
	Iterator End()
	{
		return Iterator(Data() + m_Used);
	}

	//! ConstIterator to the last element in the array
	/**
	\return The element beyond the last in the array
	*/
	ConstIterator End() const
	{
		return EndC();
	}

	ConstIterator EndC() const
	{
		return ConstIterator(Data() + m_Used);
	}
	//! Iterator to the first element in the array
	/**
	\return The element before the first in the array
	*/
	Iterator Begin()
	{
		return Iterator(Data() - 1);
	}

	//! ConstIterator to the first element in the array
	/**
	\return The element before the first in the array
	*/
	ConstIterator Begin() const
	{
		return ConstIterator(Data() - 1);
	}

	//! Iterator to the last element in the array
	/**
	\return The last element in the array
	*/
	Iterator Last()
	{
		return Iterator(Data() + m_Used - 1);
	}

	//! ConstIterator to the last element in the array
	/**
	\return The last element in the array
	*/
	ConstIterator Last() const
	{
		return ConstIterator(Data() + m_Used - 1);
	}

	//! The i-th element from the back of the array
	const T& Back(size_t i = 0) const
	{
		lxAssert(i < m_Used);
		return Data()[m_Used - i - 1];
	}

	//! The i-th element from the back of the array
	T& Back(size_t i = 0)
	{
		lxAssert(i < m_Used);
		return Data()[m_Used - i - 1];
	}

	//! The i-th element from the front of the array
	const T& Front(size_t i = 0) const
	{
		lxAssert(i < m_Used);
		return Data()[i];
	}

	//! The i-th element from the front of the array
	T& Front(size_t i = 0)
	{
		lxAssert(i < m_Used);
		return Data()[i];
	}

	//! Swap the content of two interators
	void Swap(Iterator a, Iterator b)
	{
		T entry = std::move(*a);
		*a = std::move(*b);
		*b = entry;
	}

	//! Checks if the list is empty
	/**
	\return Is the list empty
	*/
	bool IsEmpty() const
	{
		return (m_Used == 0);
	}
	//! Get a pointer to the internal data
	/**
	\return A pointer to the internal data
	*/
	T* Data()
	{
		return (T*)m_Data;
	}

	//! Get a pointer to the internal data(constant version)
	/**
	\return A pointer to the internal data
	*/
	const T* Data() const
	{
		return (const T*)m_Data;
	}

	//! Get a pointer to the internal data(constant version)
	/**
	\return A pointer to the internal data
	*/
	const T* Data_c() const
	{
		return (const T*)m_Data;
	}
	//! The number of used elements
	/**
	\return The size of the array
	*/
	size_t Size() const
	{
		return m_Used;
	}
	//! The number of allocated elements
	/**
	\return The max size of the array, before reallocation
	*/
	size_t Allocated() const
	{
		return m_Alloc;
	}

	//! Access a single element
	const T& operator[](size_t entry) const
	{
		lxAssert(entry < m_Used);
		return Data()[entry];
	}

	//! Access a single element
	T& operator[](size_t entry)
	{
		lxAssert(entry < m_Used);
		return Data()[entry];
	}

	//! Support for foreach loop
	Iterator begin()
	{
		return Iterator(Data());
	}

	//! Support for foreach loop
	ConstIterator begin() const
	{
		return ConstIterator(Data());
	}

	//! Support for foreach loop
	Iterator end()
	{
		return Iterator(Data() + m_Used);
	}

	//! Support for foreach loop
	ConstIterator end() const
	{
		return ConstIterator(Data() + m_Used);
	}

	//! Access entry with check for array size
	/**
	\throws OutOfRangeException
	*/
	const T& At(size_t entry) const
	{
		if(entry >= m_Used)
			throw core::OutOfRangeException();

		return Data()[entry];
	}

	//! Access entry with check for array size
	/**
	\throws OutOfRangeException
	*/
	T& At(size_t entry)
	{
		if(entry >= m_Used)
			throw core::OutOfRangeException();

		return Data()[entry];
	}

private:
	static void* ArrayAllocate(size_t bytes)
	{
		return ::operator new(bytes);
	}
	static void ArrayFree(void* ptr)
	{
		::operator delete(ptr);
	}

	T* GetInsertPointer(Iterator before, bool destroy = false)
	{
		lxAssert(before.m_Current - Data() <= (int)m_Used);

		const size_t pos = before.m_Current - Data();

		if(m_Used == m_Alloc)
			Reserve((m_Used * 3) / 2 + 1);

		// Shift each element, after the insert one back
		// Starting at the last element.
		if(m_Used != pos) {
			// Construct the last element
			new ((void*)(Data() + m_Used)) T(std::move(Data()[m_Used - 1]));

			// Move the remaining
			for(size_t i = m_Used - 1; i > pos; --i)
				Data()[i] = std::move(Data()[i - 1]);
		}

		T* ptr = Data() + pos;
		if(destroy && pos < m_Used)
			ptr->~T();
		++m_Used;
		return ptr;
	}

private:
	void* m_Data = nullptr;
	size_t m_Used = 0;
	size_t m_Alloc = 0;
};

LUX_API const char* MakeArrayTypeName(Type baseType);

class AbstractArrayTypeInfo
{
public:
	virtual ~AbstractArrayTypeInfo() {}
	virtual void* At(void* ptr, size_t i) const = 0;
	virtual const void* AtConst(const void* ptr, size_t i) const = 0;
	virtual size_t Size(const void* ptr) const = 0;
	virtual void Resize(void* ptr, size_t size) const = 0;
	virtual Type GetBaseType() const = 0;
};

template <typename T>
class ArrayTypeInfo : public TypeInfoTemplate<Array<T>>, public AbstractArrayTypeInfo
{
public:
	using ArrayT = Array<T>;
	ArrayTypeInfo(Type baseType) :
		TypeInfoTemplate(MakeArrayTypeName(baseType)),
		m_BaseType(baseType)
	{
	}

	Type GetBaseType() const
	{
		return m_BaseType;
	}

	void* At(void* ptr, size_t i) const
	{
		return &static_cast<ArrayT*>(ptr)->At(i);
	}
	const void* AtConst(const void* ptr, size_t i) const
	{
		return &static_cast<const ArrayT*>(ptr)->At(i);
	}
	size_t Size(const void* ptr) const
	{
		return static_cast<const ArrayT*>(ptr)->Size();
	}
	void Resize(void* ptr, size_t size) const
	{
		return static_cast<ArrayT*>(ptr)->Resize(size);
	}

private:
	Type m_BaseType;
};

template <typename T>
struct TemplType<Array<T>>
{
	static ArrayTypeInfo<T> typeInfo;
	static Type Get()
	{
		return Type(&typeInfo);
	}
};

template <typename T>
ArrayTypeInfo<T> TemplType<Array<T>>::typeInfo(TemplType<T>::Get());

namespace Types
{
template <typename T>
Type Array()
{
	return TemplType<Array<T>>::Get();
}

inline bool IsArray(Type type)
{
	return dynamic_cast<const AbstractArrayTypeInfo*>(type.GetInfo()) != nullptr;
}

inline Type GetArrayBase(Type type)
{
	auto aati = dynamic_cast<const AbstractArrayTypeInfo*>(type.GetInfo());
	if(aati)
		return aati->GetBaseType();
	return Type::Unknown;
}

} // namespace Types

template <typename T>
struct HashType<Array<T>>
{
	size_t operator()(const Array<T>& arr) const
	{
		if(arr.IsEmpty())
			return 0;

		size_t out = 7;
		const T* ptr = arr.Data();
		const T* end = ptr + arr.Size();
		HashType<T> hasher;
		for(; ptr != end; ++ptr)
			out = 31 * out + hasher(*ptr);
		return out;
	}
};

LUX_API Type TestCall();

} // namespace core
} // namespace lux

#endif
