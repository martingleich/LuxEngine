#ifndef INCLUDED_LUX_SORT_H
#define INCLUDED_LUX_SORT_H
#include "core/iterators/lxBaseIterator.h"
#include "core/lxUtil.h"

namespace lux
{
namespace core
{

// Versenkt ein element im Heap
template <typename RanIt, typename Compare>
inline void Heapsink(RanIt data, int elem, int max, const Compare& compare)
{
	while((elem << 1) < max) {
		int j = elem << 1;
		if(j + 1 < max && compare.Smaller(*(data + j), *(data + j + 1)))
			j = j + 1;

		if(compare.Smaller(*(data + elem), *(data + j))) {
			auto tmp(std::move(*(data + j)));
			*(data + j) = std::move(*(data + elem));
			*(data + elem) = std::move(tmp);

			elem = j;
		} else
			return;
	}
}

//! Sort a rand iterator range with heapsort
template <typename RanIt, typename Compare>
inline void Heapsort(RanIt first, RanIt end, const Compare& compare)
{
	static_assert(std::is_same<typename _iter_traits<RanIt>::_iter_cat, core::RandomAccessIteratorTag>::value,
		"Heapsort can only sort randomaccess iterators.");

	auto size = core::IteratorDistance(first, end);
	if(size <= 1)
		return;

	//ElementType* virtualData = data - 1;
	auto virtualFirst = first - 1;
	auto virtualSize = size + 2;
	int i;
	for(i = (size - 1) / 2; i != ((int)0-1); --i)
		Heapsink(virtualFirst, i + 1, virtualSize - 1, compare);

	for(i = size - 1; i > 0; --i) {
		auto tmp(std::move(*first));
		*first = std::move(*(first + i));
		*(first + i) = std::move(tmp);
		Heapsink(virtualFirst, 1, i + 1, compare);
	}
}

template <typename RangeT, typename CompareType>
void Sort(RangeT&& range, CompareType compare)
{
	using namespace std;
	Heapsort(begin(range), end(range), compare);
}

}
}

#endif
