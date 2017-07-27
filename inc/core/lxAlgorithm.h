#ifndef INCLUDED_LXALGORITHM_H
#define INCLUDED_LXALGORITHM_H 
#include "lxSort.h"

namespace lux
{
namespace core
{

//! Performs a linear search for an element, with userdefined condition
/**
\param from The forward iterator to start the search, must be before to
\param to The forward iterator which limits the search, must be after from
\param entry The entry to scan for
\param Con A binary function with the interface (*Iterator, entry)->bool,
returns true if the entry matches otherwise false
\return An iterator to the searched entry if found, otherwise from
*/
template <typename RangeT, typename type, typename Condition>
decltype(std::declval<RangeT>().begin()) LinearSearch(const type& entry, RangeT&& range, Condition con)
{
	auto it = range.first();
	for(; it != range.end(); ++it) {
		if(con(*it, entry))
			break;
	}

	return it;
}

//! Performs a linear search for an element
/**
The operation
*Iterator == type, must be defined
\param from The forward iterator to start the search, must be before to
\param to The forward iterator which limits the search, must be after from
\param entry The entry to scan for
\return An iterator to the searched entry if found, otherwise from
*/
template <typename RangeT, typename type>
decltype(std::declval<RangeT>().begin()) LinearSearch(const type& entry, RangeT&& range)
{
	// Lineare Suche
	auto it = range.begin();
	for(; it != range.end(); ++it) {
		if(*it == entry)
			break;
	}

	return it;
}
#if 0
//! Performs a reverse linear search for an element
/**
The operation (*Iterator == type), must be defined

\param from The forward-iterator which limits the search, must be before to
\param to The forward-iterator to start the search, must be after from
\param entry The entry to scan for
\return An iterator to the searched entry if found, otherwise from
*/
template <typename Iterator, typename type>
Iterator ReverseLinearSearch(const type& entry, Iterator from, Iterator to)
{
	// Lineare Suche
	for(; to != from; --to) {
		if(*to == entry)
			break;
	}

	return from;
}

//! Performs a reverse linear search for an element, with userdefined condition
/**
\param from The forward-iterator which limits the search, must be before to
\param to The forward-iterator to start the search, must be after from
\param entry The entry to scan for
\param Con A binary function with the interface (*Iterator, entry)->bool,
returns true if the entry matches otherwise false
\return An iterator to the searched entry if found, otherwise from
*/
template <typename Iterator, typename type, typename Condition>
Iterator ReverseLinearSearch(const type& entry, Iterator from, Iterator to, Condition con)
{
	// Lineare Suche
	for(; to != from; --to) {
		if(Con(*to, entry))
			break;
	}

	return to;
}
#endif

//! Performs an action for each element
/**
Performs an action for each element between two iterators
\param from The forward-iterator to the first element, must be before to
\param to The forward-iterator which limits the elements, must be after from
\param Action An unary function, taking an *Iterator, which is called for each element
*/
template <typename RangeT, typename Action>
void Foreach(RangeT&& range, Action act)
{
	auto it = range.begin();
	for(; it != range.end(); ++it) {
		act(*it);
	}
}

//! Performs a binary search in an sorted array
/**
\param entry The entry to search
\param begin The first interator which is searched.
\param end The interator after the last interator which is searched.
\param outNewEntry If not null and the entry was not found here the Interator where it should placed to keep the array sorted is written.
\return The iterator to the search interator or end if it couldn't be found.
*/
template <typename RangeT, typename T, typename IterT = decltype(std::declval<RangeT>().begin())>
IterT BinarySearch(const T& entry, RangeT&& range, IterT* outNextEntry = nullptr)
{
	auto begin = range.begin();
	auto end = range.end();

	if(begin == end) {
		if(outNextEntry)
			*outNextEntry = end;
		return end;
	}

	IterT left = begin;
	IterT right = end - 1;
	IterT middle = left;
	while(IteratorDistance(left, right) >= 0) {
		middle = AdvanceIterator(left, IteratorDistance(left, right) / 2);
		if(*middle == entry)
			return middle;
		else if(entry < *middle)
			right = middle - 1;
		else
			left = middle + 1;
	}

	if(outNextEntry) {
		if(entry < *middle)
			*outNextEntry = middle;
		else
			*outNextEntry = middle + 1;
	}

	return end;
}

//! Delete all elements fullfilling a condition
/**
The order of the not removed elements is preserved.
After the call the squence from first to the return value, only
contains elements not fullfilling the condition.
\param first The begin of the sequence
\param end The element after the last in the sequence
\param predicate The condition, returns true to delete a element.
\return The end iterator of the new sequence.
*/
template <typename RangeT, typename Predicate, typename IterT = decltype(std::declval<RangeT>().begin())>
IterT RemoveIf(RangeT&& range, Predicate pred)
{
	IterT first = range.begin();
	IterT cursor = first;
	IterT end = range.end();
	while(first != end) {
		if(!pred(*first)) {
			if(cursor != first)
				*cursor = std::move(*first);
			++cursor;
		}
		++first;
	}

	return cursor;
}

//! Delete all elements equal to a value
/**
The order of the not removed elements is preserved.
\param first The begin of the sequence
\param end The element after the last in the sequence
\param value The value to remove
\return The end iterator of the new sequence.
*/
template <typename RangeT, typename ValueT, typename IterT = decltype(std::declval<RangeT>().begin())>
IterT Remove(RangeT&& range, ValueT& value)
{
	IterT first = range.begin();
	IterT end = range.end();
	IterT cursor = first;
	while(first != end) {
		if(*first != value) {
			if(cursor != first)
				*cursor = std::move(*first);
			++cursor;
		}
		++first;
	}

	return cursor;
}

//! Fill the range between two iterators with a given value.
/**
\param first The first iterator of the fill range.
\param end The end iterator of the fill range.
\param v The value to fill with.
*/
template <typename RangeT, typename Value>
void Fill(RangeT&& range, Value v)
{
	auto first = range.begin();
	auto end = range.end();
	while(first != end) {
		*first = v;
		++first;
	}
}

}
}

#endif