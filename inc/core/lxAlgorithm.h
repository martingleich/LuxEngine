#ifndef INCLUDED_LUX_ALGORITHM_H
#define INCLUDED_LUX_ALGORITHM_H 
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
template <typename RangeT, typename T, typename ConditionT>
auto LinearSearch(const T& entry, RangeT&& range, ConditionT&& con)
{
	using namespace std;
	auto it = begin(range);
	auto endIt = end(range);
	for(; it != endIt; ++it) {
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
template <typename RangeT, typename T>
auto LinearSearch(const T& entry, RangeT&& range)
{
	using namespace std;
	// Lineare Suche
	auto it = begin(range);
	auto endIt = end(range);
	for(; it != endIt; ++it) {
		if(*it == entry)
			break;
	}

	return it;
}

//! Performs an action for each element
/**
Performs an action for each element between two iterators
\param from The forward-iterator to the first element, must be before to
\param to The forward-iterator which limits the elements, must be after from
\param Action An unary function, taking an *Iterator, which is called for each element
*/
template <typename RangeT, typename ActionT>
void Foreach(RangeT&& range, ActionT&& act)
{
	using namespace std;
	auto it = begin(range);
	auto endIt = end(range);
	for(; it != endIt; ++it)
		act(*it);
}

//! Performs a binary search in an sorted array
/**
\param entry The entry to search
\param range The range to search
\param outNewEntry If not null and the entry was not found here the Interator where it should placed to keep the array sorted is written.
\return The iterator to the search interator or end if it couldn't be found.
*/
template <typename RangeT, typename T, typename IterT = decltype(begin(std::declval<RangeT>()))>
IterT BinarySearch(const T& entry, RangeT&& range, IterT* outNextEntry = nullptr)
{
	using namespace std;
	auto beginIt = begin(range);
	auto endIt = end(range);

	if(beginIt == endIt) {
		if(outNextEntry)
			*outNextEntry = endIt;
		return endIt;
	}

	IterT left = beginIt;
	IterT right = endIt - 1;
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

	return endIt;
}

//! Delete all elements fullfilling a condition
/**
The order of the not removed elements is preserved.
The sequence is not resized after this operation, use the return value for this.
\param range The range of elements
\param predicate The condition, returns true to delete a element.
\return The size of the new sequence
*/
template <typename RangeT, typename Predicate>
int RemoveIf(RangeT&& range, Predicate&& pred)
{
	using namespace std;
	int newSize = 0;
	auto first = begin(range);
	auto cursor = first;
	auto endIt = end(range);
	while(first != endIt) {
		if(!pred(*first)) {
			if(cursor != first)
				*cursor = std::move(*first);
			++cursor;
			++newSize;
		}
		++first;
	}

	return newSize;
}

//! Delete all elements equal to a value
/**
The order of the not removed elements is preserved.
The sequence is not resized after this operation, use the return value for this.
\param range The range of elements
\param value The value to remove
\return The size of the new sequence
*/
template <typename RangeT, typename ValueT>
int Remove(RangeT&& range, const ValueT& value)
{
	using namespace std;
	using IterValueT = decltype(*std::begin(range));
	return RemoveIf(range, [&value](const IterValueT& v) { return v == value; });
}

//! Fill the range between two iterators with a given value.
/**
\param range The range to fill
\param v The value to fill with.
*/
template <typename RangeT, typename ValueT>
void Fill(RangeT&& range, const ValueT& v)
{
	using namespace std;
	auto first = begin(range);
	auto endIt = end(range);
	while(first != endIt) {
		*first = v;
		++first;
	}
}

}
}

#endif