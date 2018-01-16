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
	static_assert(__is_enum(T), "Type is not supported for hashes.");
};

template <typename T>
struct HashType<T*> : BitWiseHash<T*>
{};

template <>
struct HashType<u32> : BitWiseHash<u32>
{};

template <>
struct HashType<u16> : BitWiseHash<u16>
{};

template <>
struct HashType<u8> : BitWiseHash<u8>
{};

template <>
struct HashType<s32> : BitWiseHash<u32>
{};

template <>
struct HashType<s16> : BitWiseHash<u16>
{};

template <>
struct HashType<s8> : BitWiseHash<u8>
{};

template <>
struct HashType<char> : BitWiseHash<char>
{};

template <>
struct HashType<wchar_t> : BitWiseHash<wchar_t>
{};

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

}
}

#endif // !INCLUDED_LX_UTIL_H