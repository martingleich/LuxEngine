#ifndef INCLUDED_LUX_BASE_ITERATOR_H
#define INCLUDED_LUX_BASE_ITERATOR_H 
#include <stdlib.h>
#include <utility>
#include <xutility>

namespace lux
{
namespace core
{

///\cond INTERNAL
struct ForwardIteratorTag {};
struct BidirectionalIteratorTag : ForwardIteratorTag {};
struct RandomAccessIteratorTag : BidirectionalIteratorTag {};

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
	if(distance > 0) {
		while(distance > 0) {
			++iter;
			--distance;
		}
	} else {
		while(distance < 0) {
			--iter;
			++distance;
		}
	}


	return iter;
}

template <typename IterType>
IterType _IteratorAdvance(IterType iter, int distance, ForwardIteratorTag)
{
	lxAssert(distance >= 0);
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

#define LX_MAKE_BASE_BI_ITER(Iterator, ItState, type) \
class Base##Iterator : public lux::core::BaseIterator<lux::core::BidirectionalIteratorTag, type>\
{ \
public: \
	Base##Iterator() = default; \
	Base##Iterator(ItState state) : m_State(state) {} \
	const ItState& GetState() const { return m_State; } \
protected: \
	ItState m_State; \
}; \
 \
class Iterator : public Base##Iterator \
{ \
public: \
	Iterator() = default; \
	explicit Iterator(ItState state) : Base##Iterator(state) {} \
	Iterator& operator++() { m_State.next(); return *this; } \
	Iterator& operator--() { m_State.prev(); return *this; } \
	Iterator operator++(int) { Iterator tmp(*this); m_State.next(); return tmp; } \
	Iterator operator--(int) { Iterator tmp(*this); m_State.prev(); return tmp; } \
 \
	bool operator==(const Base##Iterator& other) const { return m_State.cmp(other.GetState()); } \
	bool operator!=(const Base##Iterator& other) const { return !m_State.cmp(other.GetState()); } \
 \
	type& operator*() { return m_State.get_ref(); } \
	const type& operator*() const { return m_State.get_const(); } \
 \
	type* operator->() { return &m_State.get_ref(); } \
	const type* operator->() const { return &m_State.get_const(); } \
}; \
 \
class Const##Iterator : public Base##Iterator \
{ \
public: \
	Const##Iterator() = default; \
	explicit Const##Iterator(ItState state) : Base##Iterator(state) { } \
	Const##Iterator(const Iterator& other) : Base##Iterator(other.GetState()) { } \
	Const##Iterator& operator=(const Iterator& other) { m_State = other.GetState(); return *this; } \
 \
	Const##Iterator& operator++() { m_State.next(); return *this; } \
	Const##Iterator& operator--() { m_State.prev(); return *this; } \
	Const##Iterator operator++(int) { Const##Iterator tmp(*this); m_State.next(); return tmp; } \
	Const##Iterator operator--(int) { Const##Iterator tmp(*this); m_State.prev(); return tmp; } \
 \
	bool operator==(const Base##Iterator& other) const { return m_State.cmp(other.GetState()); } \
	bool operator!=(const Base##Iterator& other) const { return !m_State.cmp(other.GetState()); } \
 \
	const type& operator*() const { return m_State.get_const(); } \
	const type* operator->() const { return &m_State.get_const(); } \
};

} // namespace core
} // namespace lux

#endif
