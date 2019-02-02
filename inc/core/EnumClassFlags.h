#ifndef INCLUDED_LUX_ENUMCLASSFLAGS_H
#define INCLUDED_LUX_ENUMCLASSFLAGS_H

namespace lux
{
/*
These function should be used when enum classes are use as bit flags.
*/

template<typename T>
constexpr bool TestFlag(T a, T b)
{
	return (int(a)&int(b))!=0;
}
template<typename T>
T& SetFlag(T& a, T b)
{
	return (a = T(int(a) | int(b)));
}
template<typename T>
T& ClearFlag(T& a, T b)
{
	return (a = T(int(a)&~int(b)));
}
template<typename T>
T& FlipFlag(T&a, T b)
{
	return (a = T(int(a) ^ int(b)));
}
template <typename T>
constexpr T CombineFlags(T a, T b)
{
	return T(int(a) | int(b));
}
template <typename T>
constexpr T CombineFlags(T a, T b, T c)
{
	return T(int(a) | int(b) | int(c));
}
template <typename T>
constexpr T CombineFlags(T a, T b, T c, T d)
{
	return T(int(a) | int(b) | int(c) | int(d));
}
template <typename T>
constexpr T CombineFlags(T a, T b, T c, T d, T e)
{
	return T(int(a) | int(b) | int(c) | int(d) | int(e));
}

} // namespace lux

#endif // INCLUDED_LUX_ENUMCLASSFLAGS_H