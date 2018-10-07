#ifndef INCLUDED_LUX_ENUMCLASSFLAGS_H
#define INCLUDED_LUX_ENUMCLASSFLAGS_H
#include <type_traits>
#include <cstddef>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace lux
{

template <typename T>
struct is_flag_type
{
	static const bool value = std::is_integral<T>::value || std::is_enum<T>::value;
};

template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type operator~ (T a)
{
	return static_cast<T>(~static_cast<int>(a));
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type operator| (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type operator& (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type operator^ (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) ^ static_cast<int>(b));
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type& operator|= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	return a;
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type& operator&= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
	return a;
}
template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, T>::type& operator^= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) ^ static_cast<int>(b));
	return a;
}

template<typename T> constexpr typename std::enable_if<is_flag_type<T>::value, bool>::type TestFlag(T a, T b)
{
	return (static_cast<int>(a&b) != 0);
}
template<typename T> typename std::enable_if<is_flag_type<T>::value, T>::type& SetFlag(T& a, T b)
{
	return (a |= b);
}
template<typename T> typename std::enable_if<is_flag_type<T>::value, T>::type& ClearFlag(T& a, T b)
{
	return (a &= ~b);
}
template<typename T> inline typename std::enable_if<is_flag_type<T>::value, T>::type& FlipFlag(T&a, T b)
{
	return (a ^= b);
}

template<typename T> inline int GetFlagId(T a)
{
	unsigned long value = static_cast<unsigned long>(a);
	unsigned long index;
#ifdef _MSC_VER
	_BitScanForward(&index, value);
#else
	index = 0;
	while(value & 1) {
		value >>= 1;
		++index;
	}
#endif
	return (int)index;
}

template<typename T> inline int GetFlagSetCount(T a)
{
	unsigned int value = static_cast<unsigned int>(a);
	int count;
#ifdef _MSC_VER
	count = (int)__popcnt(value);
#else
	for(count = 0; value; ++count)
		value &= value - 1;
#endif
	return count;
}


}

#endif // INCLUDED_LUX_ENUMCLASSFLAGS_H