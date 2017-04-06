#ifndef INCLUDED_LXITERATOR_H
#define INCLUDED_LXITERATOR_H 

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
size_t _IteratorDistance(IterType a, IterType b, RandomAccessIteratorTag)
{
	return b - a;
}

template <typename IterType>
size_t _IteratorDistance(IterType a, IterType b, BidirectionalIteratorTag)
{
	s32 count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
size_t _IteratorDistance(IterType a, IterType b, InputIteratorTag)
{
	size_t count = 0;
	while(a != b) {
		count++;
		a++;
	}

	return count;
}

template <typename IterType>
size_t _IteratorDistance(IterType a, IterType b, ForwardIteratorTag)
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

}
}

#endif