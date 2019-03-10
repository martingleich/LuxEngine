#ifndef INCLUDED_LX_INDEX_ITERATOR_H
#define INCLUDED_LX_INDEX_ITERATOR_H
#include "core/iterators/lxRanges.h"
#include "core/iterators/lxMultiIterator.h"

namespace lux
{
namespace core
{

template <typename BaseT>
class IndexIter : public BaseIterator<RandomAccessIteratorTag, BaseT>
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

	const BaseT* operator->() const { return &value; }
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
auto IndexedRange(RangeT&& range)
{
	using namespace std;
	auto indexRange = core::MakeIndexRange();
	auto a = ZipIter(begin(std::forward<RangeT>(range)), indexRange.begin());
	auto b = ZipIter(end(std::forward<RangeT>(range)), indexRange.end());
	return MakeRange(a, b);
}

} // namespace core
} // namespace lux
#endif // #ifndef INCLUDED_LX_INDEX_ITERATOR_H

