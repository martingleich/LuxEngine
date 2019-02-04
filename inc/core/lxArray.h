#ifndef INCLUDED_LUX_LXARRAY_H
#define INCLUDED_LUX_LXARRAY_H
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
public:
	using Iterator = T * ;
	using ConstIterator = const T*;

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
		*this = init;
	}

	//! Destruktor
	~Array()
	{
		Destroy();
	}

	//! Copyconstructor
	Array(const Array<T>& other)
	{
		*this = other;
	}

	//! Move constructor
	Array(Array<T>&& old)
	{
		m_Data = old.m_Data;
		m_Used = old.m_Used;
		m_Alloc = old.m_Alloc;
		old.m_Data = nullptr;
		old.m_Used = 0;
		old.m_Alloc = 0;
	}

	//! Move assignment 
	Array<T>& operator=(Array<T>&& old)
	{
		if(this != &old) {
			std::swap(m_Data, old.m_Data);
			std::swap(m_Used, old.m_Used);
			std::swap(m_Alloc, old.m_Alloc);
		}

		return *this;
	}

	//! Assignment
	Array<T>& operator=(const Array<T>& other)
	{
		if(this == &other)
			return *this;

		// Alte Einträge löschen
		Clear();

		// Neu reservieren
		Reserve(other.Size());

		// Alles kopieren
		m_Used = other.m_Used;

		for(int i = 0; i < m_Used; ++i)
			new (m_Data + i) T(other.m_Data[i]);

		return *this;
	}

	Array<T>& operator=(std::initializer_list<T> init)
	{
		Clear();
		Reserve((int)init.size());
		for(int i = 0; i < (int)init.size(); ++i)
			new (m_Data + i) T(init.begin()[i]);
		m_Used = (int)init.size();

		return *this;
	}

	//! Comparison
	bool operator==(const Array<T>& other) const
	{
		if(Size() != other.Size())
			return false;
		for(int i = 0; i < other.Size(); ++i) {
			if(m_Data[i] != other.m_Data[i])
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
	void Insert(const T& entry, int before)
	{
		// Copy if reference point to inside the container.
		auto ref = &entry;
		alignas(alignof(T)) const char* copyData[sizeof(T)];
		if(&entry >= Data() && &entry < Data() + m_Alloc) {
			new (copyData) T(entry);
			ref = (const T*)copyData;
		}
		auto ptr = GetInsertPointer(before);
		if(ptr == Data() + Size() - 1)
			new (ptr) T(*ref);
		else
			*ptr = *ref;
	}

	//! Add a new entry at any position to the array(move version)
	/**
	\param entry The new entry
	\param before entry is build at this position
	*/
	void Insert(T&& entry, int before)
	{
		// Copy if reference point to inside the container.
		auto ref = &entry;
		alignas(alignof(T)) const char* copyData[sizeof(T)];
		if(&entry >= Data() && &entry < Data() + m_Alloc) {
			new (copyData) T(std::move(entry));
			ref = (T*)copyData;
		}
		auto ptr = GetInsertPointer(before);
		if(ptr == Data() + m_Used - 1)
			new (ptr) T(std::move(*ref));
		else
			*ptr = std::move(*ref);
	}

	template <typename... Ts>
	T& Emplace(int before, Ts&&... args)
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
		Insert(entry, m_Used);
	}

	//! Add a new entry to the end of the array(move version)
	/**
	\param entry The entry to add
	*/
	void PushBack(T&& entry)
	{
		Insert((T&&)entry, m_Used);
	}

	template <typename... Ts>
	T& EmplaceBack(Ts&&... args)
	{
		auto ptr = GetInsertPointer(m_Used);
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
		Insert(entry, 0);
	}

	//! Insert a new entry a the beginning of the array(move version)
	/**
	Very slow operation, the whole array have to be moved
	\param entry The entry to insert
	*/
	void PushFront(T&& entry)
	{
		Insert(entry, 0);
	}

	template <typename... Ts>
	T& EmplaceFront(Ts&&... args)
	{
		auto ptr = GetInsertPointer(0, true);
		new (ptr) T(std::forward<Ts>(args)...);
		return *ptr;
	}

	void PushBack(const core::Array<T>& entries)
	{
		PushBack(entries.Data_c(), entries.Size());
	}

	//! Remove the last element in the array
	void PopBack()
	{
		EraseHoldOrder(m_Used - 1);
	}

	//! Remove the last element in the array
	void PopFront()
	{
		EraseHoldOrder(0);
	}

	//! Remove an entry in the array
	/**
	The order of the array may be changed.
	\param it The element to remove
	*/
	void Erase(int pos)
	{
		BasicErase(pos, 1, false);
	}
	//! Remove an entry in the array
	/**
	\param it The element to remove
	*/
	void EraseHoldOrder(int pos)
	{
		BasicErase(pos, 1, true);
	}

	//! Remove multiple elements from the array
	/**
	The order of the array may be changed.
	\param from The first element to remove.
	\param count The number of elements to remove.
	*/
	void Erase(int from, int count)
	{
		BasicErase(from, count, false);
	}

	//! Remove multiple elements from the array
	/**
	\param from The first element to remove.
	\param count The number of elements to remove.
	*/
	void EraseHoldOrder(int from, int count)
	{
		BasicErase(from, count, true);
	}

	//! Remove all elements from the list
	void Clear()
	{
		for(int i = 0; i < m_Used; ++i)
			m_Data[i].~T();

		m_Used = 0;
		m_Alloc = 0;
	}

	//! Release unused memory
	void ShrinkToFit()
	{
		if(m_Used != m_Alloc)
			ForceReserve(m_Used);
	}

	//! Set the reserved size of the list
	void Reserve(int newAlloc)
	{
		if(newAlloc > m_Alloc)
			ForceReserve(newAlloc);
	}

	//! Set the number of used elements aka the size of the array
	/**
	Only used elements can be accessed via [] operator
	\param used The new size of the list
	\param defaultValue The value of new elements.
	*/
	void Resize(int used, const T& defaultValue)
	{
		if(m_Alloc < used) {
			Reserve(used);
		} else {
			for(int i = used; i < m_Used; ++i)
				Data()[i].~T();
		}

		for(int i = m_Used; i < used; ++i)
			new ((void*)&Data()[i]) T(defaultValue);

		m_Used = used;
	}

	//! Set the number of used elements aka the size of the array
	/**
	Only used elements can be accessed via [] operator.
	New elements are default constructed
	\param used The new size of the list
	*/
	void Resize(int used)
	{
		if(m_Alloc < used) {
			Reserve(used);
			for(int i = m_Used; i < used; ++i)
				new ((void*)&Data()[i]) T();
		} else {
			for(int i = used; i < m_Used; ++i)
				Data()[i].~T();
		}

		for(int i = m_Used; i < used; ++i)
			new ((void*)&Data()[i]) T();

		m_Used = used;
	}

	//! Shrink only resize.
	/**
	Doesn't require default construcible elements.
	*/
	void ShrinkResize(int used)
	{
		lxAssert(m_Alloc >= used);
		for(int i = used; i < m_Used; ++i)
			Data()[i].~T();
		m_Used = used;
	}

	//! Iterator to the first element in the array
	/**
	\return The first element in the array
	*/
	Iterator First()
	{
		return Iterator(m_Data);
	}

	//! ConstIterator to the first element in the array
	/**
	\return The first element in the array
	*/
	ConstIterator First() const
	{
		return ConstIterator(m_Data);
	}

	//! Iterator to the last element in the array
	/**
	\return The element beyond the last in the array
	*/
	Iterator End()
	{
		return Iterator(m_Data + m_Used);
	}

	//! ConstIterator to the last element in the array
	/**
	\return The element beyond the last in the array
	*/
	ConstIterator End() const
	{
		return ConstIterator(m_Data + m_Used);
	}

	//! The i-th element from the back of the array
	const T& Back(int i = 0) const { return At(m_Used - i - 1); }

	//! The i-th element from the back of the array
	T& Back(int i = 0) { return At(m_Used - i - 1); }

	//! The i-th element from the front of the array
	const T& Front(int i = 0) const { return At(i); }

	//! The i-th element from the front of the array
	T& Front(int i = 0) { return At(i); }

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
		return m_Data;
	}

	//! Get a pointer to the internal data(constant version)
	/**
	\return A pointer to the internal data
	*/
	const T* Data() const
	{
		return m_Data;
	}

	//! Get a pointer to the internal data(constant version)
	/**
	\return A pointer to the internal data
	*/
	const T* Data_c() const
	{
		return m_Data;
	}
	//! The number of used elements
	/**
	\return The size of the array
	*/
	int Size() const
	{
		return m_Used;
	}
	//! The number of allocated elements
	/**
	\return The max size of the array, before reallocation
	*/
	int Allocated() const
	{
		return m_Alloc;
	}

	//! Access a single element
	const T& operator[](int entry) const
	{
		lxAssert(entry >= 0 || entry < m_Used);
		return Data()[entry];
	}

	//! Access a single element
	T& operator[](int entry)
	{
		lxAssert(entry >= 0 || entry < m_Used);
		return Data()[entry];
	}

	Iterator begin() { return Iterator(m_Data); }
	ConstIterator begin() const { return ConstIterator(m_Data); }
	Iterator end() { return Iterator(m_Data + m_Used); }
	ConstIterator end() const { return ConstIterator(m_Data + m_Used); }

	//! Access entry with check for array size
	/**
	\throws OutOfRangeException
	*/
	const T& At(int entry) const
	{
		LX_CHECK_BOUNDS(entry, 0, m_Used);
		return Data()[entry];
	}

	//! Access entry with check for array size
	/**
	\throws OutOfRangeException
	*/
	T& At(int entry)
	{
		LX_CHECK_BOUNDS(entry, 0, m_Used);
		return Data()[entry];
	}

	///////////////////////////////////////////////////////
	// Algorithms

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

	template <typename ElemT>
	int LinearSearch(const ElemT& elem)
	{
		for(int i = 0; i < m_Used; ++i) {
			if(m_Data[i] == elem)
				return i;
		}
		return -1;
	}

	template <typename Predicate>
	void RemoveIf(Predicate pred)
	{
		int first = 0;
		int cursor = first;
		int end = m_Used;
		while(first != end) {
			if(!pred(m_Data[first])) {
				if(cursor != first)
					m_Data[cursor] = std::move(m_Data[first]);
				++cursor;
			}
			++first;
		}
		ShrinkResize(cursor);
	}


private:
	static void* ArrayAllocate(int bytes)
	{
		return ::operator new(bytes);
	}
	static void ArrayFree(void* ptr)
	{
		::operator delete(ptr);
	}

	void BasicErase(int from, int count, bool holdOrder)
	{
		lxAssert(from >= 0 && from + count - 1 <= m_Used);

		if(holdOrder) {
			for(int i = from; i < m_Used - count; ++i)
				m_Data[i] = std::move(m_Data[i + count]);
		} else {
			auto offset = m_Used - count - from;
			auto toCopy = math::Min(count, offset);

			for(int i = 0; i < toCopy; ++i)
				m_Data[from + i] = std::move(m_Data[offset + i]);
		}

		for(int i = 0; i < count; ++i)
			m_Data[m_Used - 1 - i].~T();

		m_Used -= count;
	}
	int GetNextSize(int minSize)
	{
		int next = (m_Alloc * 3) / 2;
		if(next > minSize)
			return next;
		return minSize;
	}
	T* GetInsertPointer(int before, bool destroy = false)
	{
		lxAssert(before >= 0 && before <= m_Used);

		if(m_Used == m_Alloc)
			Reserve(GetNextSize(m_Used + 1));

		// Shift each element, after the insert one back
		// Starting at the last element.
		if(m_Used != before) {
			// Construct the last element
			new ((void*)(Data() + m_Used)) T(std::move(Data()[m_Used - 1]));

			// Move the remaining
			for(int i = m_Used - 1; i > before; --i)
				Data()[i] = std::move(Data()[i - 1]);
		}

		T* ptr = Data() + before;
		if(destroy && before < m_Used)
			ptr->~T();
		++m_Used;
		return ptr;
	}

	void Destroy()
	{
		Clear();
		ArrayFree(Data());
		m_Data = nullptr;
	}

	void ForceReserve(int newAlloc)
	{
		lxAssert(newAlloc >= m_Used);
		T* newEntries = (T*)ArrayAllocate(newAlloc * sizeof(T));
		for(int i = 0; i < m_Used; ++i) {
			new ((void*)&newEntries[i]) T(std::move(m_Data[i]));
			m_Data[i].~T();
		}
		ArrayFree(m_Data);

		m_Data = newEntries;
		m_Alloc = newAlloc;
	}


private:
	T* m_Data = nullptr;
	int m_Used = 0;
	int m_Alloc = 0;
};

template <typename T>
inline typename Array<T>::Iterator begin(Array<T>& array) { return array.begin(); }
template <typename T>
inline typename Array<T>::Iterator end(Array<T>& array) { return array.end(); }

template <typename T>
inline typename Array<T>::ConstIterator begin(const Array<T>& array) { return array.begin(); }
template <typename T>
inline typename Array<T>::ConstIterator end(const Array<T>& array) { return array.end(); }

LUX_API StringView MakeArrayTypeName(Type baseType);

template <typename T>
void fmtPrint(format::Context& ctx, const core::Array<T>& array, format::Placeholder& placeholder)
{
	using namespace format;
	ctx.AddTerminatedSlice("[");
	for(int i = 0; i < array.Size(); ++i) {
		fmtPrint(ctx, array[i], placeholder);
		if(i < array.Size() - 1)
			ctx.AddTerminatedSlice(", ");
	}
	ctx.AddTerminatedSlice("]");
}

class AbstractArrayTypeInfo
{
public:
	virtual ~AbstractArrayTypeInfo() {}
	virtual void* At(void* ptr, int i) const = 0;
	virtual const void* AtConst(const void* ptr, int i) const = 0;
	virtual int Size(const void* ptr) const = 0;
	virtual void Resize(void* ptr, int size) const = 0;
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

	void* At(void* ptr, int i) const
	{
		return &static_cast<ArrayT*>(ptr)->At(i);
	}
	const void* AtConst(const void* ptr, int i) const
	{
		return &static_cast<const ArrayT*>(ptr)->At(i);
	}
	int Size(const void* ptr) const
	{
		return static_cast<const ArrayT*>(ptr)->Size();
	}
	void Resize(void* ptr, int size) const
	{
		return static_cast<ArrayT*>(ptr)->Resize(size);
	}
	bool EqualInfo(const TypeInfo* otherInfo) const
	{
		auto other = dynamic_cast<ArrayTypeInfo*>(otherInfo);
		if(!other)
			return false;
		return m_BaseType == other.m_BaseType;
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
	unsigned int operator()(const Array<T>& arr) const
	{
		const T* ptr = arr.Data();
		SequenceHasher seqHasher;
		HashType<T> hasher;
		for(int i = 0; i < arr.Size(); ++i)
			seqHasher.Add(hasher(ptr[i]));
		return seqHasher.GetHash();
	}
};

} // namespace core
} // namespace lux

#endif
