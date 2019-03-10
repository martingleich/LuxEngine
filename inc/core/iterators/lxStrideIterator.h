#ifndef INCLUDED_LX_STRIDE_ITERATOR
#define INCLUDED_LX_STRIDE_ITERATOR
#include "core/iterators/lxRanges.h"

namespace lux
{
namespace core
{

template <typename T, bool IsConst>
class StrideBaseIterator : public core::BaseIterator<core::RandomAccessIteratorTag, T>
{
public:
	using VoidPtrT = typename core::Choose<IsConst, const void*, void*>::type;
	using BytePtrT = typename core::Choose<IsConst, const u8*, u8*>::type;
	using ElemPtrT = typename core::Choose<IsConst, const T*, T*>::type;

	StrideBaseIterator() :
		m_Stride(0),
		m_Ptr(nullptr)
	{
	}

	//! Create a stride iterator.
	/**
	\param ptr Pointer to data.
	\param stride Distance between two objects in byte.
	\param off Offset of the first object from the data pointer in bytes.
	*/
	StrideBaseIterator(VoidPtrT ptr, int stride, int off = 0) :
		m_Stride(stride),
		m_Ptr((BytePtrT)ptr + off)
	{
	}

	StrideBaseIterator& operator+=(int i)
	{
		m_Ptr += i*m_Stride;
		return *this;
	}
	StrideBaseIterator& operator-=(int i)
	{
		return (*this += -i);
	}
	StrideBaseIterator operator+(int i) const
	{
		auto tmp = *this;
		tmp += i;
		return tmp;
	}
	StrideBaseIterator operator-(int i) const
	{
		auto tmp = *this;
		tmp -= i;
		return tmp;
	}
	int operator-(StrideBaseIterator& other) const
	{
		return (int)(m_Ptr - other.m_Ptr) / m_Stride;
	}

	StrideBaseIterator& operator++()
	{
		m_Ptr += m_Stride;
		return *this;
	}
	StrideBaseIterator& operator--()
	{
		m_Ptr -= m_Stride;
		return *this;
	}

	StrideBaseIterator operator++(int)
	{
		auto tmp = *this;
		++*this;
		return tmp;
	}
	StrideBaseIterator operator--(int)
	{
		auto tmp = *this;
		--*this;
		return tmp;
	}

	template <bool IsConst2>
	bool operator==(const StrideBaseIterator<T, IsConst2>& other) const
	{
		return m_Ptr == other.m_Ptr;
	}

	template <bool IsConst2>
	bool operator!=(const StrideBaseIterator<T, IsConst2>& other) const
	{
		return !(*this == other);
	}

	template <bool U = IsConst, std::enable_if_t<U, int> = 0>
	const T& operator*() const
	{
		return *((ElemPtrT)m_Ptr);
	}

	template <bool U = IsConst, std::enable_if_t<U, int> = 0>
	const T* operator->() const
	{
		return ((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T& operator*() const
	{
		return *((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T* operator->() const
	{
		return ((ElemPtrT)m_Ptr);
	}

	int m_Stride;
	BytePtrT m_Ptr;
};

//! Iterator based on distance between elements.
template <typename T>
using StrideIterator = StrideBaseIterator<T, false>;

//! Constant iterator based on distance between elements.
template <typename T>
using ConstStrideIterator = StrideBaseIterator<T, true>;

template <typename T, bool IsConst>
class BaseStrideRange : public Range<StrideBaseIterator<T, IsConst>>
{
public:
	BaseStrideRange(StrideBaseIterator<T, IsConst> f, StrideBaseIterator<T, IsConst> e) :
		Range(f, e)
	{
	}

	int Size() const
	{
		return IteratorDistance(First(), End());
	}

	const T& operator[](int i) const
	{
		return *AdvanceIterator(First(), i);
	}
	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T& operator[](int i)
	{
		return *AdvanceIterator(First(), i);
	}
};

//! A range of stride iterators.
template <typename T>
using StrideRange = BaseStrideRange<T, false>;

//! A constant range of stride iterators.
template <typename T>
using ConstStrideRange = BaseStrideRange<T, true>;

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_STRIDE_ITERATOR
