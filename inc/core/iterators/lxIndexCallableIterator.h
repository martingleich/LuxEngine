#ifndef INCLUDED_LX_INDEX_CALLABLE_ITERATOR_H
#define INCLUDED_LX_INDEX_CALLABLE_ITERATOR_H
#include "core/iterators/lxRanges.h"

namespace lux
{
namespace core
{

template <typename ValueT, typename Callable, typename IndexT = int>
class IndexCallableIteratorRefReturn : public BaseIterator<RandomAccessIteratorTag, ValueT>
{
public:
	IndexCallableIteratorRefReturn()
	{
	}

	IndexCallableIteratorRefReturn(IndexT index, const Callable& callable) :
		m_Index(index),
		m_Callable(callable)
	{
	}

	IndexCallableIteratorRefReturn& operator++() { ++m_Index; return *this; }
	IndexCallableIteratorRefReturn& operator--() { --m_Index; return *this; }
	IndexCallableIteratorRefReturn operator++(int)
	{
		IndexCallableIteratorRefReturn tmp(*this); ++m_Index; return tmp;
	}
	IndexCallableIteratorRefReturn operator--(int)
	{
		IndexCallableIteratorRefReturn tmp = *this; --m_Index; return tmp;
	}

	IndexCallableIteratorRefReturn& operator+=(int num)
	{
		m_Index += num;
		return *this;
	}

	IndexCallableIteratorRefReturn operator+(int num) const
	{
		IndexCallableIteratorRefReturn temp = *this; return temp += num;
	}
	IndexCallableIteratorRefReturn& operator-=(int num)
	{
		return (*this) += (-num);
	}
	IndexCallableIteratorRefReturn operator-(int num) const
	{
		return (*this) + (-num);
	}

	int operator-(const IndexCallableIteratorRefReturn& other) const
	{
		return m_Index - other.m_Index;
	}

	bool operator==(const IndexCallableIteratorRefReturn& other) const
	{
		return m_Index == other.m_Index;
	}
	bool operator!=(const IndexCallableIteratorRefReturn& other) const
	{
		return m_Index != other.m_Index;
	}

	const ValueT& operator*() const
	{
		return m_Callable(m_Index);
	}

	const ValueT* operator->() const
	{
		return &m_Callable(m_Index);
	}

private:
	IndexT m_Index;
	Callable m_Callable;
};

template <typename ValueT, typename Callable, typename IndexT = int>
class IndexCallableIteratorValueReturn : public BaseIterator<RandomAccessIteratorTag, ValueT>
{
public:
	IndexCallableIteratorValueReturn()
	{
	}

	IndexCallableIteratorValueReturn(IndexT index, const Callable& callable) :
		m_Index(index),
		m_Callable(callable)
	{
	}

	void SetIndex(int i)
	{
		m_Index = i;
		m_HasValue = false;
	}
	IndexCallableIteratorValueReturn& operator++() { SetIndex(m_Index+1); return *this; }
	IndexCallableIteratorValueReturn& operator--() { SetIndex(m_Index-1); return *this; }
	IndexCallableIteratorValueReturn operator++(int)
	{
		IndexCallableIteratorValueReturn tmp(*this); SetIndex(m_Index+1); return tmp;
	}
	IndexCallableIteratorValueReturn operator--(int)
	{
		IndexCallableIteratorValueReturn tmp = *this; SetIndex(m_Index-1); return tmp;
	}

	IndexCallableIteratorValueReturn& operator+=(int num)
	{
		SetIndex(m_Index+num);
		return *this;
	}

	IndexCallableIteratorValueReturn operator+(int num) const
	{
		IndexCallableIteratorValueReturn temp = *this;
		return temp += num;
	}
	IndexCallableIteratorValueReturn& operator-=(int num)
	{
		return (*this) += (-num);
	}
	IndexCallableIteratorValueReturn operator-(int num) const
	{
		return (*this) + (-num);
	}

	int operator-(const IndexCallableIteratorValueReturn& other) const
	{
		return m_Index - other.m_Index;
	}

	bool operator==(const IndexCallableIteratorValueReturn& other) const
	{
		return m_Index == other.m_Index;
	}
	bool operator!=(const IndexCallableIteratorValueReturn& other) const
	{
		return m_Index != other.m_Index;
	}

	const ValueT& operator*() const
	{
		if(!m_HasValue) {
			m_Value = m_Callable(m_Index);
			m_HasValue = true;
		}
		return m_Value;
	}

	const ValueT* operator->() const
	{
		return &(*(*this));
	}

private:
	IndexT m_Index;
	Callable m_Callable;
	mutable ValueT m_Value;
	mutable bool m_HasValue=false;
};

template <typename ValueT, typename CallableT, typename IndexT = int>
Range<IndexCallableIteratorRefReturn<ValueT, CallableT, IndexT>> MakeIndexCallableRangeRefReturn(CallableT&& callable, IndexT startIndex, IndexT endIndex)
{
	auto b = IndexCallableIteratorRefReturn<ValueT, CallableT, IndexT>(startIndex, callable);
	auto e = IndexCallableIteratorRefReturn<ValueT, CallableT, IndexT>(endIndex, callable);
	return MakeRange(b, e);
}

template <typename ValueT, typename CallableT, typename IndexT = int>
Range<IndexCallableIteratorValueReturn<ValueT, CallableT, IndexT>> MakeIndexCallableRangeValueReturn(CallableT&& callable, IndexT startIndex, IndexT endIndex)
{
	auto b = IndexCallableIteratorValueReturn<ValueT, CallableT, IndexT>(startIndex, callable);
	auto e = IndexCallableIteratorValueReturn<ValueT, CallableT, IndexT>(endIndex, callable);
	return MakeRange(b, e);
}

} // namespace core
} // namespace lux

#endif // #ifndef INCLUDED_LX_INDEX_CALLABLE_ITERATOR_H

