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

inline unsigned int IntHashFunc(unsigned int x)
{
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

inline unsigned int InverseIntHashFunc(unsigned int x)
{  
	x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
	return x;
}

// FNV-1a Hasher
struct SequenceHasher
{
	void Add(unsigned int value)
	{
		hash ^= value;
		hash *= 16777619;
	}
	unsigned int GetHash() const { return hash; }
private:
	unsigned int hash = 2166136261;
};

inline unsigned int HashSequence(const void* _ptr, int size)
{
	SequenceHasher hasher;
	const u8* ptr = (u8*)_ptr;
	for(int i = 0; i < size; ++i)
		hasher.Add(ptr[i]);
	return hasher.GetHash();
}

template <typename T>
struct BitWiseHash
{
	unsigned int operator()(const T& t) const
	{
		ifconst(sizeof(t) <= sizeof(int)) {
			unsigned int bits;
			std::memcpy(&bits, &t, sizeof(T));
			return IntHashFunc(bits);
		} else {
			return HashSequence(reinterpret_cast<const u8*>(&t), sizeof(T));
		}
	}
};

template <typename T>
struct HashType
{
	typename std::enable_if<std::is_enum<T>::value, unsigned int>::type operator()(T t) const
	{
		return IntHashFunc((unsigned int)t);
	}
};

template <typename T>
struct MemberFuncHashType
{
	unsigned int operator()(const T& t) { return t.GetHash(); }
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
	unsigned int operator()(float x)
	{
		return BitWiseHash<float>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<double> : BitWiseHash<double>
{
	unsigned int operator()(double x)
	{
		return BitWiseHash<double>::operator()(x == -0 ? +0 : x);
	}
};

template <>
struct HashType<long double> : BitWiseHash<long double>
{
	unsigned int operator()(long double x)
	{
		return BitWiseHash<long double>::operator()(x == -0 ? +0 : x);
	}
};

}
}

#endif // !INCLUDED_LUX_LX_UTIL_H