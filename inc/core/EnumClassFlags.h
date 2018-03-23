#ifndef INCLUDED_LUX_ENUMCLASSFLAGS_H
#define INCLUDED_LUX_ENUMCLASSFLAGS_H
#include <type_traits>
#include <cstddef>

namespace lux
{

template <typename T>
struct is_flag_enum
{
	static const bool value = std::is_integral<T>::value || std::is_enum<T>::value;
};

#define DEFINE_FLAG_ENUM_CLASS(name) \
enum class name; \
template <>\
struct is_flag_enum<name>\
{\
static const bool value = true;\
};\
enum class name

#define DECLARE_FLAG_CLASS(name)\
template <>\
struct is_flag_enum<name>\
{\
static const bool value = true;\
};

template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type operator~ (T a)
{
	return static_cast<T>(~static_cast<int>(a));
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type operator| (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type operator& (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type operator^ (T a, T b)
{
	return static_cast<T>(static_cast<int>(a) ^ static_cast<int>(b));
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& operator|= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	return a;
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& operator&= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
	return a;
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& operator^= (T& a, T b)
{
	a = static_cast<T>(static_cast<int>(a) ^ static_cast<int>(b));
	return a;
}

template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, bool>::type TestFlag(T a, T b)
{
	return (static_cast<int>(a&b) != 0);
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& SetFlag(T& a, T b)
{
	return (a |= b);
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& ClearFlag(T& a, T b)
{
	return (a &= ~b);
}
template<typename T> inline typename std::enable_if<is_flag_enum<T>::value, T>::type& FlipFlag(T&a, T b)
{
	return (a ^= b);
}

}

#endif // INCLUDED_LUX_ENUMCLASSFLAGS_H