#ifndef INCLUDED_LXITERATOR_H
#define INCLUDED_LXITERATOR_H 
#include <stdint.h>
#include <stdlib.h>
#include <utility>

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
};

template <typename IterType>
struct _iter_traits<IterType*>
{
	typedef RandomAccessIteratorTag _iter_cat;
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

template <typename IterT>
class Range
{
public:
	Range(IterT f, IterT e) :
		m_First(f),
		m_End(e)
	{}

	IterT First()
	{
		return m_First;
	}

	IterT begin()
	{
		return m_First;
	}

	IterT End()
	{
		return m_End;
	}

	IterT end()
	{
		return m_End;
	}

private:
	IterT m_First;
	IterT m_End;
};

}
}

#endif