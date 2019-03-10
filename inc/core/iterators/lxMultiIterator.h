#ifndef INCLUDED_LX_MULTI_ITER_H
#define INCLUDED_LX_MULTI_ITER_H
#include "core/iterators/lxBaseIterator.h"
#include <tuple>

namespace lux
{
namespace core
{

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

template <typename... RangeTs>
auto ZipRange(RangeTs&&... ranges)
{
	using namespace std;
	auto a = ZipIter(begin(std::forward<RangeTs>(ranges))...);
	auto b = ZipIter(end(std::forward<RangeTs>(ranges))...);
	return MakeRange(a, b);
}

} // namespace core
} // namespace lux
#endif // #ifndef INCLUDED_LX_MULTI_ITER_H

