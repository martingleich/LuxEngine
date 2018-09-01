#ifndef INCLUDED_LUX_LX_UTIL_H
#define INCLUDED_LUX_LX_UTIL_H
#include "LuxBase.h"

namespace lux
{
namespace core
{

template <typename T>
struct CompareType
{
	bool Equal(const T& a, const T& b) const { return a == b; }
	bool Smaller(const T& a, const T& b) const { return a < b; }
};

template <typename T, typename FuncT>
struct CompareTypeFromSmallerT
{
	FuncT func;
	CompareTypeFromSmallerT(const FuncT& smallerFunc) :
		func(smallerFunc)
	{}
	bool Equal(const T& a, const T& b) const { return !func(a, b) && !func(b, a); }
	bool Smaller(const T& a, const T& b) const { return func(a, b); }
};

template <typename T, typename FuncT>
struct CompareTypeFromIntCompareT
{
	FuncT func;
	CompareTypeFromIntCompareT(const FuncT& intFunc) :
		func(intFunc)
	{}
	bool Equal(const T& a, const T& b) const { return func(a,b) == 0; }
	bool Smaller(const T& a, const T& b) const { return func(a,b) < 0; }
};


template <typename T, typename FuncT>
CompareTypeFromSmallerT<T, FuncT> CompareTypeFromSmaller(const FuncT& func)
{
	return CompareTypeFromSmallerT<T, FuncT>(func);
}
template <typename T, typename FuncT>
CompareTypeFromIntCompareT<T, FuncT> CompareTypeFromInt(const FuncT& func)
{
	return CompareTypeFromIntCompareT<T, FuncT>(func);
}

inline int HashSequence(const u8* ptr, int size)
{
	int out = 7;
	const u8* end = ptr + size;
	for(; ptr != end; ++ptr)
		out = 31 * out + *ptr;
	return out;
}

template <typename T>
struct BitWiseHash
{
	int operator()(const T& t) const
	{
		return HashSequence(reinterpret_cast<const u8*>(&t), sizeof(T));
	}
};

template <typename T>
struct HashType
{
	typename std::enable_if<std::is_enum<T>::value, int>::type operator()(T t) const
	{
		return (int)t;
	}
};

template <typename T>
struct HashType<T*> : BitWiseHash<T*> {};

template <> struct HashType<char> : BitWiseHash<char> {};
template <> struct HashType<wchar_t> : BitWiseHash<wchar_t> {};
template <> struct HashType<signed char> : BitWiseHash<signed char> {};
template <> struct HashType<unsigned char> : BitWiseHash<unsigned char> {};
template <> struct HashType<signed short> : BitWiseHash<signed short> {};
template <> struct HashType<unsigned short> : BitWiseHash<unsigned short> {};
template <> struct HashType<signed int> : BitWiseHash<signed int> {};
template <> struct HashType<unsigned int> : BitWiseHash<unsigned int> {};
template <> struct HashType<signed long> : BitWiseHash<signed long> {};
template <> struct HashType<unsigned long> : BitWiseHash<unsigned long> {};
template <> struct HashType<signed long long> : BitWiseHash<signed long long> {};
template <> struct HashType<unsigned long long> : BitWiseHash<unsigned long long> {};

template <>
struct HashType<float> : BitWiseHash<float>
{
	int operator()(float x)
	{
		return BitWiseHash<float>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<double> : BitWiseHash<double>
{
	int operator()(double x)
	{
		return BitWiseHash<double>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<long double> : BitWiseHash<long double>
{
	int operator()(long double x)
	{
		return BitWiseHash<long double>::operator()(x == -0 ? +0 : x);
	}
};

}
}

#endif // !INCLUDED_LUX_LX_UTIL_H