#ifndef INCLUDED_LX_UTIL_H
#define INCLUDED_LX_UTIL_H
#include "LuxBase.h"

namespace lux
{
namespace core
{

template <typename T>
struct CompareType
{
	bool Equal(const T& a, const T& b) const
	{
		return a == b;
	}

	bool Smaller(const T& a, const T& b) const
	{
		return a < b;
	}
};

inline size_t HashSequence(const u8* ptr, size_t size)
{
	size_t out = 7;
	const u8* end = ptr + size;
	for(; ptr != end; ++ptr)
		out = 31 * out + *ptr;
	return out;
}

template <typename T>
struct BitWiseHash
{
	size_t operator()(const T& t) const
	{
		return HashSequence(reinterpret_cast<const u8*>(&t), sizeof(T));
	}
};

template <typename T>
struct HashType : BitWiseHash<T>
{
	std::enable_if<std::is_enum<T>::value, size_t> operator()(T t) const
	{
		return (size_t)t;
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
	size_t operator()(float x)
	{
		return BitWiseHash<float>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<double> : BitWiseHash<double>
{
	size_t operator()(double x)
	{
		return BitWiseHash<double>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<long double> : BitWiseHash<long double>
{
	size_t operator()(long double x)
	{
		return BitWiseHash<long double>::operator()(x == -0 ? +0 : x);
	}
};

}
}

#endif // !INCLUDED_LX_UTIL_H