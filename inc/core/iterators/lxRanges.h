#ifndef INCLUDED_LX_RANGES_H
#define INCLUDED_LX_RANGES_H
#include "core/iterators/lxBaseIterator.h"

namespace lux
{
namespace core
{

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

}
}

#endif // #ifndef INCLUDED_LX_RANGES_H

