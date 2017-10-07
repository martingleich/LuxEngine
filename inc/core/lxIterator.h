#ifndef INCLUDED_LXITERATOR_H
#define INCLUDED_LXITERATOR_H 
#include <stdint.h>
#include <stdlib.h>
#include <utility>
#include <tuple>

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
intptr_t _IteratorDistance(IterType a, IterType b, RandomAccessIteratorTag)
{
	return b - a;
}

template <typename IterType>
intptr_t _IteratorDistance(IterType a, IterType b, BidirectionalIteratorTag)
{
	intptr_t count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
intptr_t _IteratorDistance(IterType a, IterType b, InputIteratorTag)
{
	size_t count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
intptr_t _IteratorDistance(IterType a, IterType b, ForwardIteratorTag)
{
	size_t count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, size_t distance, RandomAccessIteratorTag)
{
	return (iter + distance);
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, size_t distance, BidirectionalIteratorTag)
{
	while(distance > 0) {
		++iter;
		--distance;
	}

	return iter;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, size_t distance, InputIteratorTag)
{
	while(distance > 0) {
		++iter;
		--distance;
	}

	return iter;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, size_t distance, ForwardIteratorTag)
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
intptr_t IteratorDistance(IterType a, IterType b)
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
IterType AdvanceIterator(IterType a, size_t distance)
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
	static const size_t TupleSize = sizeof...(Ts);
	using TupleType = std::tuple<Ts...>;

	struct DerefType
	{
		using TupleType = std::tuple<decltype(*std::declval<Ts>())...>;
		DerefType(TupleType&& old) :
			m_Tuple(std::forward<TupleType>(old))
		{
		}

		template <size_t Index>
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

	template <size_t Index>
	typename std::tuple_element<Index, TupleType>::type get() { return std::get<Index>(m_Tuple); }

	DerefType operator*()
	{
		return DerefImpl(m_Tuple, std::make_index_sequence<TupleSize>());
	}

private:
	template <size_t... Indices>
	static void IncImpl(TupleType& tuple, std::integer_sequence<size_t, Indices...>)
	{
		int ignored[] = {(std::get<Indices>(tuple)++, 0)...};
		(void)ignored;
	}

	template <typename TupleT, size_t... Indices>
	static DerefType DerefImpl(TupleT&& tuple, std::integer_sequence<size_t, Indices...>)
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

template <typename IterT>
class Range
{
public:
	Range(IterT f, IterT e) :
		m_First(f),
		m_End(e)
	{
	}

	IterT First() const
	{
		return m_First;
	}

	IterT begin() const
	{
		return m_First;
	}

	IterT End() const
	{
		return m_End;
	}

	IterT end() const
	{
		return m_End;
	}

private:
	IterT m_First;
	IterT m_End;
};

template <typename RangeT>
size_t RangeLength(const RangeT& r)
{
	using namespace std;
	return IteratorDistance(begin(r), end(r));
}

template <typename IterT>
Range<IterT> MakeRange(IterT first, IterT end)
{
	return Range<IterT>(first, end);
}

template <typename BaseT>
struct IndexIter : public BaseIterator<RandomAccessIteratorTag, BaseT>
{
public:
	IndexIter() {}
	IndexIter(BaseT v) : value(v) {}
	bool operator==(IndexIter other) const { return value == other.value; }
	bool operator!=(IndexIter other) const { return value != other.value; }
	IndexIter operator++() {++value; return *this; }
	IndexIter& operator+=(intptr_t v) { value += (BaseT)v; return *this; }
	IndexIter operator+(intptr_t v) { return IndexIter(value + (BaseT)v); }
	IndexIter operator++(int) {IndexIter tmp(*this); ++value; return tmp; }
	IndexIter operator--() {--value; return *this; }
	IndexIter& operator-=(intptr_t v) { value -= (BaseT)v; return *this; }
	IndexIter operator-(intptr_t v) { return IndexIter(value - (BaseT)v); }
	IndexIter operator--(int) {IndexIter tmp(*this); --value; return tmp; }

	intptr_t operator-(IndexIter other) { return value-other.value; }

	BaseT operator*() const { return value; }

private:
	BaseT value;
};

template <typename IndexT = size_t>
Range<IndexIter<IndexT>> MakeIndexRange(IndexT end = std::numeric_limits<IndexT>::max())
{
	return Range<IndexIter<IndexT>>(0, end);
}

template <typename IndexT = size_t>
Range<IndexIter<IndexT>> MakeIndexRange(IndexT start, IndexT end)
{
	return Range<IndexIter<IndexT>>(start, end);
}

template <typename RangeT, typename IterT = decltype(std::declval<RangeT>().begin())>
Range<IterT> SliceRange(RangeT&& range, intptr_t first, intptr_t end = -1)
{
	using namespace std;
	size_t len = RangeLength(std::forward<RangeT>(range));
	if(end < 0)
		end = len + end + 1;
	lxAssert(end >= first);
	IterT itFirst = AdvanceIterator(begin(range), first);
	IterT itEnd = AdvanceIterator(itFirst, end - first);
	return Range<IterT>(itFirst, itEnd);
}

template <typename RangeT, typename IterT = decltype(std::declval<RangeT>().begin())>
Range<IterT> SliceRangeCount(RangeT&& range, intptr_t first, intptr_t count)
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

}
}

#endif