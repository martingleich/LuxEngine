#ifndef INCLUDED_LXITERATOR_H
#define INCLUDED_LXITERATOR_H 
#include <stdint.h>
#include <stdlib.h>
#include <utility>
#include <xutility>
#include <tuple>
#include <limits>

namespace lux
{
namespace core
{

///\cond INTERNAL
struct InputIteratorTag
{};
struct ForwardIteratorTag : InputIteratorTag
{};
struct BidirectionalIteratorTag : ForwardIteratorTag
{};
struct RandomAccessIteratorTag : BidirectionalIteratorTag
{};

template <typename IterType>
struct _iter_traits
{
	typedef typename IterType::IteratorCategory _iter_cat;
	typedef typename IterType::ValueType _value_type;
};

template <typename ValueType>
struct _iter_traits<ValueType*>
{
	typedef RandomAccessIteratorTag _iter_cat;
	typedef ValueType _value_type;
};

template <typename _itercat, typename _valueType>
struct BaseIterator
{
	typedef _itercat IteratorCategory;
	typedef _valueType ValueType;
};

template <typename IterType>
typename _iter_traits<IterType>::_iter_cat _iterCat(IterType& it)
{
	LUX_UNUSED(it);
	typename _iter_traits<IterType>::_iter_cat x;
	return x;
}

template <typename IterType>
int _IteratorDistance(IterType a, IterType b, RandomAccessIteratorTag)
{
	return static_cast<int>(b - a);
}

template <typename IterType>
int _IteratorDistance(IterType a, IterType b, BidirectionalIteratorTag)
{
	int count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
int _IteratorDistance(IterType a, IterType b, InputIteratorTag)
{
	int count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
int _IteratorDistance(IterType a, IterType b, ForwardIteratorTag)
{
	int count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, int distance, RandomAccessIteratorTag)
{
	return (iter + distance);
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, int distance, BidirectionalIteratorTag)
{
	while(distance > 0) {
		++iter;
		--distance;
	}

	return iter;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, int distance, InputIteratorTag)
{
	while(distance > 0) {
		++iter;
		--distance;
	}

	return iter;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, int distance, ForwardIteratorTag)
{
	while(distance > 0) {
		++iter;
		--distance;
	}

	return iter;
}

///\endcond

//! The distance between two iterators
/**
a must be before b
The distance is defined as the smallest value x, with (a += x) == b
\param a The first iterator
\param b The second iterator
\return The distance between the iterators
*/
template <typename IterType>
int IteratorDistance(IterType a, IterType b)
{
	return _IteratorDistance(a, b, _iterCat(a));
}

//! Advances an interator with a given amount
/**
The operation is equivalent to repeated inkrement of the interator
\param a The interator to advance.
\param distance The distance to advance the iterator.
*/
template <typename IterType>
IterType AdvanceIterator(IterType a, int distance)
{
	return _IteratorAdvance(a, distance, _iterCat(a));
}

template <typename IterType>
struct IterValueType
{
	typedef typename _iter_traits<IterType>::_value_type type;
};

//! Swap the content of two interators
/**
\param a The first iterator.
\param b The second iterator.
*/
template <typename IterType>
void SwapIterator(IterType& a, IterType& b)
{
	auto tmp(std::move(*a));
	*a = std::move(*b);
	*b = std::move(tmp);
}

template <typename... Ts>
class MultiIter
{
	static const int TupleSize = sizeof...(Ts);
	using TupleType = std::tuple<Ts...>;

	struct DerefType
	{
		using TupleType = std::tuple<decltype(*std::declval<Ts>())...>;
		DerefType(TupleType&& old) :
			m_Tuple(std::forward<TupleType>(old))
		{
		}

		template <int Index>
		typename std::tuple_element<Index, TupleType>::type get() { return std::get<Index>(m_Tuple); }

	private:
		TupleType m_Tuple;
	};

public:
	MultiIter(Ts... ts) :
		m_Tuple(ts...)
	{
	}

	MultiIter& operator++()
	{
		IncImpl(m_Tuple, std::make_index_sequence<TupleSize>());
		return *this;
	}
	MultiIter operator++(int)
	{
		MultiIter copy(*this);
		IncImpl(m_Tuple, std::make_index_sequence<TupleSize>());
		return copy;
	}

	bool operator==(const MultiIter& other) const
	{
		return std::get<0>(m_Tuple) == std::get<0>(other.m_Tuple);
	}

	bool operator!=(const MultiIter& other) const
	{
		return !(*this == other);
	}

	template <int Index>
	typename std::tuple_element<Index, TupleType>::type get() { return std::get<Index>(m_Tuple); }

	DerefType operator*()
	{
		return DerefImpl(m_Tuple, std::make_index_sequence<TupleSize>());
	}

private:
	template <int... Indices>
	static void IncImpl(TupleType& tuple, std::integer_sequence<int, Indices...>)
	{
		int ignored[] = {(std::get<Indices>(tuple)++, 0)...};
		(void)ignored;
	}

	template <typename TupleT, int... Indices>
	static DerefType DerefImpl(TupleT&& tuple, std::integer_sequence<int, Indices...>)
	{
		return std::forward_as_tuple((*std::get<Indices>(tuple))...);
	}

private:
	TupleType m_Tuple;
};

template <typename... IterTs>
MultiIter<IterTs...> ZipIter(IterTs... iter)
{
	return MultiIter<IterTs...>(iter...);
}

template <typename RangeT>
int RangeLength(const RangeT& r)
{
	using namespace std;
	return IteratorDistance(begin(r), end(r));
}

template <typename IterT>
class Range
{
public:
	Range() {}
	Range(IterT f, IterT e) :
		m_First(f),
		m_End(e)
	{
	}

	IterT First() const
	{
		return m_First;
	}

	IterT End() const
	{
		return m_End;
	}

	bool operator==(const Range& other) const
	{
		return m_First == other.m_First && m_End == other.m_End;
	}
	bool operator!=(const Range& other) const
	{
		return !(*this == other);
	}

	bool IsEmpty() const
	{
		return m_First == m_End;
	}

private:
	IterT m_First;
	IterT m_End;
};

template <typename IterT>
IterT begin(const Range<IterT>& range) { return range.First(); }
template <typename IterT>
IterT end(const Range<IterT>& range) { return range.End(); }

template <typename IterT>
Range<IterT> MakeRange(IterT first, IterT end)
{
	return Range<IterT>(first, end);
}

template <typename Class>
auto MakeRange(Class&& obj)
{
	using namespace std;
	return MakeRange(begin(obj), end(obj));
}

template <typename BaseT>
struct IndexIter : public BaseIterator<RandomAccessIteratorTag, BaseT>
{
public:
	IndexIter() {}
	IndexIter(BaseT v) : value(v) {}
	bool operator==(IndexIter other) const { return value == other.value; }
	bool operator!=(IndexIter other) const { return value != other.value; }
	IndexIter operator++() { ++value; return *this; }
	IndexIter& operator+=(int v) { value += (BaseT)v; return *this; }
	IndexIter operator+(int v) { return IndexIter(value + (BaseT)v); }
	IndexIter operator++(int) { IndexIter tmp(*this); ++value; return tmp; }
	IndexIter operator--() { --value; return *this; }
	IndexIter& operator-=(int v) { value -= (BaseT)v; return *this; }
	IndexIter operator-(int v) { return IndexIter(value - (BaseT)v); }
	IndexIter operator--(int) { IndexIter tmp(*this); --value; return tmp; }

	int operator-(IndexIter other) { return value - other.value; }

	BaseT operator*() const { return value; }

private:
	BaseT value;
};

template <typename IndexT = int>
Range<IndexIter<IndexT>> MakeIndexRange(IndexT end = std::numeric_limits<IndexT>::max())
{
	return Range<IndexIter<IndexT>>(0, end);
}

template <typename IndexT = int>
Range<IndexIter<IndexT>> MakeIndexRange(IndexT start, IndexT end)
{
	return Range<IndexIter<IndexT>>(start, end);
}

template <typename RangeT>
auto SliceRange(RangeT&& range, int first, int end = -1)
{
	using namespace std;
	int len = RangeLength(std::forward<RangeT>(range));
	if(end < 0)
		end = len + end + 1;
	lxAssert(end >= first);
	auto itFirst = AdvanceIterator(begin(range), first);
	auto itEnd = AdvanceIterator(itFirst, end - first);
	return MakeRange(itFirst, itEnd);
}

template <typename RangeT>
auto SliceRangeCount(RangeT&& range, int first, int count)
{
	return SliceRange(std::forward<RangeT>(range), first, first + count);
}

template <typename... RangeTs>
auto ZipRange(RangeTs&&... ranges)
{
	using namespace std;
	auto a = ZipIter(begin(std::forward<RangeTs>(ranges))...);
	auto b = ZipIter(end(std::forward<RangeTs>(ranges))...);
	return MakeRange(a, b);
}

template <typename RangeT>
auto IndexedRange(RangeT&& range)
{
	using namespace std;
	auto indexRange = core::MakeIndexRange();
	auto a = ZipIter(begin(std::forward<RangeT>(range)), indexRange.begin());
	auto b = ZipIter(end(std::forward<RangeT>(range)), indexRange.end());
	return MakeRange(a, b);
}

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
		return (m_Ptr - other.m_Ptr) / m_Stride;
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

	const T& operator*() const
	{
		return *((ElemPtrT)m_Ptr);
	}

	const T* operator->() const
	{
		return ((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T& operator*()
	{
		return *((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T* operator->()
	{
		return ((ElemPtrT)m_Ptr);
	}

	int m_Stride;
	BytePtrT m_Ptr;
};

//! Iterator based on distance between elements.
template <typename T>
using StrideIterator = StrideBaseIterator<T, false>;

template <typename T, typename Callable, typename IndexT = int>
class IndexCallableIterator : public BaseIterator<RandomAccessIteratorTag, T>
{
public:
	IndexCallableIterator()
	{
	}

	IndexCallableIterator(IndexT index, Callable callable) :
		m_Index(index),
		m_Callable(callable)
	{
	}

	IndexCallableIterator& operator++() { ++m_Index; return *this; }
	IndexCallableIterator& operator--() { --m_Index; return *this; }
	IndexCallableIterator operator++(int)
	{
		IndexCallableIterator tmp(*this); ++m_Index; return tmp;
	}
	IndexCallableIterator operator--(int)
	{
		IndexCallableIterator tmp = *this; --m_Index; return tmp;
	}

	IndexCallableIterator& operator+=(int num)
	{
		m_Index += num;
		return *this;
	}

	IndexCallableIterator operator+(int num) const
	{
		IndexCallableIterator temp = *this; return temp += num;
	}
	IndexCallableIterator& operator-=(int num)
	{
		return (*this) += (-num);
	}
	IndexCallableIterator operator-(int num) const
	{
		return (*this) + (-num);
	}

	int operator-(IndexCallableIterator other) const
	{
		return m_Index - other.m_Index;
	}

	bool operator==(const IndexCallableIterator& other) const
	{
		return m_Index == other.m_Index;
	}
	bool operator!=(const IndexCallableIterator& other) const
	{
		return m_Index != other.m_Index;
	}

	const T& operator*() const
	{
		return m_Callable(m_Index);
	}

	const T* operator->() const
	{
		return &m_Callable(m_Index);
	}

private:
	IndexT m_Index;
	Callable m_Callable;
};

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

#endif
