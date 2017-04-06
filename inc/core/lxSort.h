#ifndef INCLUDED_LXSORT_H
#define INCLUDED_LXSORT_H
#include "lxIterator.h"
#include "lxUtil.h"

namespace lux
{
namespace core
{

// Versenkt ein element im Heap
template <typename ElementType, typename Compare>
inline void Heapsink(ElementType* data, int elem, int max, const Compare& compare)
{
	// Solange es ein linkes Kind gibt
	while((elem << 1) < max) {
		int j = elem << 1;
		if(j + 1 < max && compare.Smaller(data[j], data[j + 1]))
			j = j + 1;

		if(compare.Smaller(data[elem], data[j])) {
			ElementType tmp(std::move(data[j]));
			data[j] = std::move(data[elem]);
			data[elem] = std::move(tmp);

			elem = j;
		} else
			return;
	}
}

// Sortiert ein Array mit Heapsort
template <typename ElementType, typename Compare>
inline void Heapsort(ElementType* data, size_t size, const Compare& compare)
{
	if(size <= 1)
		return;

	ElementType* virtualData = data - 1;
	int virtualSize = size + 2;
	int i;

	for(i = (size - 1) / 2; i >= 0; --i)
		Heapsink(virtualData, i + 1, virtualSize - 1, compare);

	for(i = size - 1; i > 0; --i) {
		ElementType tmp(std::move(data[0]));
		data[0] = std::move(data[i]);
		data[i] = std::move(tmp);
		Heapsink(virtualData, 1, i + 1, compare);
	}
}

// Quicksort
template <typename ElementType, typename Compare>
inline void QuickSort(ElementType* data, size_t size, const Compare& compare)
{

}


}    // namespace core
}    // namespace lux

#endif
