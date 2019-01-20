#ifndef INCLUDED_LUX_ITERATOR_H
#define INCLUDED_LUX_ITERATOR_H 
#include <stdint.h>
#include <stdlib.h>
#include <utility>
#include <xutility>
#include <tuple>
#include <limits>

namespace lux
{
namespace core
{

///\cond INTERNAL
struct ForwardIteratorTag
{};
struct BidirectionalIteratorTag : ForwardIteratorTag
{};
struct RandomAccessIteratorTag : BidirectionalIteratorTag
{};

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

template <typename... Ts>
class MultiIter
{
	static const int TupleSize = sizeof...(Ts);
	using TupleType = std::tuple<Ts...>;

	struct DerefType
	{
		using TupleType = std::tuple<decltype(*std::declval<Ts>())...>;
		DerefType(TupleType&& old) :
			m_Tuple(std::forward<TupleType>(old))
		{
		}

		template <int Index>
		typename std::tuple_element<Index, TupleType>::type get() { return std::get<Index>(m_Tuple); }

	private:
		TupleType m_Tuple;
	};

public:
	MultiIter(Ts... ts) :
		m_Tuple(ts...)
	{
	}

	MultiIter& operator++()
	{
		IncImpl(m_Tuple, std::make_index_sequence<TupleSize>());
		return *this;
	}
	MultiIter operator++(int)
	{
		MultiIter copy(*this);
		IncImpl(m_Tuple, std::make_index_sequence<TupleSize>());
		return copy;
	}

	bool operator==(const MultiIter& other) const
	{
		return std::get<0>(m_Tuple) == std::get<0>(other.m_Tuple);
	}

	bool operator!=(const MultiIter& other) const
	{
		return !(*this == other);
	}

	template <int Index>
	typename std::tuple_element<Index, TupleType>::type get() { return std::get<Index>(m_Tuple); }

	DerefType operator*()
	{
		return DerefImpl(m_Tuple, std::make_index_sequence<TupleSize>());
	}

private:
	template <int... Indices>
	static void IncImpl(TupleType& tuple, std::integer_sequence<int, Indices...>)
	{
		int ignored[] = {(std::get<Indices>(tuple)++, 0)...};
		(void)ignored;
	}

	template <typename TupleT, int... Indices>
	static DerefType DerefImpl(TupleT&& tuple, std::integer_sequence<int, Indices...>)
	{
		return std::forward_as_tuple((*std::get<Indices>(tuple))...);
	}

private:
	TupleType m_Tuple;
};

template <typename... IterTs>
MultiIter<IterTs...> ZipIter(IterTs... iter)
{
	return MultiIter<IterTs...>(iter...);
}

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

template <typename BaseT>
struct IndexIter : public BaseIterator<RandomAccessIteratorTag, BaseT>
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

template <typename... RangeTs>
auto ZipRange(RangeTs&&... ranges)
{
	using namespace std;
	auto a = ZipIter(begin(std::forward<RangeTs>(ranges))...);
	auto b = ZipIter(end(std::forward<RangeTs>(ranges))...);
	return MakeRange(a, b);
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

template <typename T>
class AbstractConstIterator
{
public:
	virtual ~AbstractConstIterator() {}
	virtual void Next() = 0;
	virtual bool Equal(const AbstractConstIterator* other) const = 0;
	virtual const T* GetPtr() const = 0;
	virtual AbstractConstIterator* Clone() const = 0;
};

template <typename BaseIterT>
class AbstractConstIteratorImplementation : public AbstractConstIterator<typename BaseIterT::ValueType>
{
public:
	using ValueType = typename BaseIterT::ValueType;
	explicit AbstractConstIteratorImplementation(BaseIterT base) :
		m_BaseIter(base)
	{
	}
	void Next() { ++m_BaseIter; }
	bool Equal(const AbstractConstIterator* other) const
	{
		return Equal(dynamic_cast<const AbstractConstIteratorImplementation*>(other));
	}
	bool Equal(const AbstractConstIteratorImplementation* other) const
	{
		return other && m_BaseIter == other->m_BaseIter;
	}
	const ValueType* GetPtr() const { return m_BaseIter.operator->(); }
	AbstractConstIterator* Clone() const { return new AbstractConstIteratorImplementation(m_BaseIter); }

private:
	BaseIterT m_BaseIter;
};

template <typename T>
class AnyBaseConstIterator : public core::BaseIterator<core::ForwardIteratorTag, T>
{
public:
	using AbstractType = AbstractConstIterator<T>;
	AnyBaseConstIterator() :
		m_Iter(nullptr)
	{
	}
	AnyBaseConstIterator(const AnyBaseConstIterator& other) :
		m_Iter(other.m_Iter ? other.m_Iter->Clone() : nullptr)
	{
	}
	AnyBaseConstIterator(AnyBaseConstIterator&& old) :
		m_Iter(old.m_Iter)
	{
		old.m_Iter = nullptr;
	}
	explicit AnyBaseConstIterator(AbstractType* ptr) :
		m_Iter(ptr)
	{
	}
	~AnyBaseConstIterator()
	{
		if(m_Iter)
			delete m_Iter;
	}
	AnyBaseConstIterator& operator=(const AnyBaseConstIterator& other)
	{
		this->~AnyBaseConstIterator();
		m_Iter = other.m_Iter ? other.m_Iter->Clone() : nullptr;
		return *this;
	}
	AnyBaseConstIterator& operator=(AnyBaseConstIterator&& old)
	{
		this->~AnyBaseIterator();
		m_Iter = old.m_Iter;
		old.m_Iter = nullptr;
		return *this;
	}
	AnyBaseConstIterator& operator++()
	{
		m_Iter->Next();
		return *this;
	}
	AnyBaseConstIterator operator++(int)
	{
		auto tmp = *this;
		++*this;
		return tmp;
	}

	bool operator==(const AnyBaseConstIterator& other) const
	{
		return m_Iter->Equal(other.m_Iter);
	}

	bool operator!=(const AnyBaseConstIterator& other) const
	{
		return !(*this == other);
	}

	const T& operator*() const { return *(m_Iter->GetPtr()); }
	const T* operator->() const { return m_Iter->GetPtr(); }

private:
	AbstractType* m_Iter;
};

template <typename IterT>
inline AnyBaseConstIterator<typename IterT::ValueType> MakeAnyConstIter(IterT it)
{
	return AnyBaseConstIterator<typename IterT::ValueType>(new AbstractConstIteratorImplementation<IterT>(it));
}

template <typename T>
using AnyRange = Range<AnyBaseConstIterator<T>>;

template <typename IterT>
inline AnyRange<typename IterT::ValueType> MakeAnyRange(IterT first, IterT end)
{
	return AnyRange<typename IterT::ValueType>(MakeAnyConstIter(first), MakeAnyConstIter(end));
}

template <typename T, bool IsConst>
class StrideBaseIterator : public core::BaseIterator<core::RandomAccessIteratorTag, T>
{
public:
	using VoidPtrT = typename core::Choose<IsConst, const void*, void*>::type;
	using BytePtrT = typename core::Choose<IsConst, const u8*, u8*>::type;
	using ElemPtrT = typename core::Choose<IsConst, const T*, T*>::type;

	StrideBaseIterator() :
		m_Stride(0),
		m_Ptr(nullptr)
	{
	}

	//! Create a stride iterator.
	/**
	\param ptr Pointer to data.
	\param stride Distance between two objects in byte.
	\param off Offset of the first object from the data pointer in bytes.
	*/
	StrideBaseIterator(VoidPtrT ptr, int stride, int off = 0) :
		m_Stride(stride),
		m_Ptr((BytePtrT)ptr + off)
	{
	}

	StrideBaseIterator& operator+=(int i)
	{
		m_Ptr += i*m_Stride;
		return *this;
	}
	StrideBaseIterator& operator-=(int i)
	{
		return (*this += -i);
	}
	StrideBaseIterator operator+(int i) const
	{
		auto tmp = *this;
		tmp += i;
		return tmp;
	}
	StrideBaseIterator operator-(int i) const
	{
		auto tmp = *this;
		tmp -= i;
		return tmp;
	}
	int operator-(StrideBaseIterator& other) const
	{
		return (int)(m_Ptr - other.m_Ptr) / m_Stride;
	}

	StrideBaseIterator& operator++()
	{
		m_Ptr += m_Stride;
		return *this;
	}
	StrideBaseIterator& operator--()
	{
		m_Ptr -= m_Stride;
		return *this;
	}

	StrideBaseIterator operator++(int)
	{
		auto tmp = *this;
		++*this;
		return tmp;
	}
	StrideBaseIterator operator--(int)
	{
		auto tmp = *this;
		--*this;
		return tmp;
	}

	template <bool IsConst2>
	bool operator==(const StrideBaseIterator<T, IsConst2>& other) const
	{
		return m_Ptr == other.m_Ptr;
	}

	template <bool IsConst2>
	bool operator!=(const StrideBaseIterator<T, IsConst2>& other) const
	{
		return !(*this == other);
	}

	template <bool U = IsConst, std::enable_if_t<U, int> = 0>
	const T& operator*() const
	{
		return *((ElemPtrT)m_Ptr);
	}

	template <bool U = IsConst, std::enable_if_t<U, int> = 0>
	const T* operator->() const
	{
		return ((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T& operator*() const
	{
		return *((ElemPtrT)m_Ptr);
	}

	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T* operator->() const
	{
		return ((ElemPtrT)m_Ptr);
	}

	int m_Stride;
	BytePtrT m_Ptr;
};

//! Iterator based on distance between elements.
template <typename T>
using StrideIterator = StrideBaseIterator<T, false>;

template <typename T, typename Callable, typename IndexT = int>
class IndexCallableIterator : public BaseIterator<RandomAccessIteratorTag, T>
{
public:
	IndexCallableIterator()
	{
	}

	IndexCallableIterator(IndexT index, Callable callable) :
		m_Index(index),
		m_Callable(callable)
	{
	}

	IndexCallableIterator& operator++() { ++m_Index; return *this; }
	IndexCallableIterator& operator--() { --m_Index; return *this; }
	IndexCallableIterator operator++(int)
	{
		IndexCallableIterator tmp(*this); ++m_Index; return tmp;
	}
	IndexCallableIterator operator--(int)
	{
		IndexCallableIterator tmp = *this; --m_Index; return tmp;
	}

	IndexCallableIterator& operator+=(int num)
	{
		m_Index += num;
		return *this;
	}

	IndexCallableIterator operator+(int num) const
	{
		IndexCallableIterator temp = *this; return temp += num;
	}
	IndexCallableIterator& operator-=(int num)
	{
		return (*this) += (-num);
	}
	IndexCallableIterator operator-(int num) const
	{
		return (*this) + (-num);
	}

	int operator-(IndexCallableIterator other) const
	{
		return m_Index - other.m_Index;
	}

	bool operator==(const IndexCallableIterator& other) const
	{
		return m_Index == other.m_Index;
	}
	bool operator!=(const IndexCallableIterator& other) const
	{
		return m_Index != other.m_Index;
	}

	const T& operator*() const
	{
		return m_Callable(m_Index);
	}

	const T* operator->() const
	{
		return &m_Callable(m_Index);
	}

private:
	IndexT m_Index;
	Callable m_Callable;
};

//! Constant iterator based on distance between elements.
template <typename T>
using ConstStrideIterator = StrideBaseIterator<T, true>;

template <typename T, bool IsConst>
class BaseStrideRange : public Range<StrideBaseIterator<T, IsConst>>
{
public:
	BaseStrideRange(StrideBaseIterator<T, IsConst> f, StrideBaseIterator<T, IsConst> e) :
		Range(f, e)
	{
	}

	int Size() const
	{
		return IteratorDistance(First(), End());
	}

	const T& operator[](int i) const
	{
		return *AdvanceIterator(First(), i);
	}
	template <bool U = !IsConst, std::enable_if_t<U, int> = 0>
	T& operator[](int i)
	{
		return *AdvanceIterator(First(), i);
	}
};

//! A range of stride iterators.
template <typename T>
using StrideRange = BaseStrideRange<T, false>;

//! A constant range of stride iterators.
template <typename T>
using ConstStrideRange = BaseStrideRange<T, true>;

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
