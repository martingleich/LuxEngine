#ifndef INCLUDED_HELPER_MACROS_H
#define INCLUDED_HELPER_MACROS_H

#define LX_DEFINE_COMPARE_FUNCTIONS_BY_SMALLER(class) \
	bool operator==(const class& other) const { return !((*this < other) || (other < *this)); } \
	bool operator!=(const class& other) const { return   (*this < other) || (other < *this); } \
	bool operator>=(const class& other) const { return !(*this < other); } \
	bool operator<=(const class& other) const { return  (*this < other) || !((*this < other) || (other < *this)); } \
	bool operator>(const class& other) const  { return !(*this < other) && ((*this < other) || (other < *this)); } \

#define LX_DEFINE_COMPARE_FUNCTIONS_BY_SMALLER_AND_EQUAL(class) \
	bool operator!=(const class& other) const { return !(*this == other); } \
	bool operator>=(const class& other) const { return !(*this < other); } \
	bool operator<=(const class& other) const { return  (*this < other) ||  (*this == other); } \
	bool operator>(const class& other) const  { return !(*this < other) && !(*this == other); } \

#define LX_MAKE_FOURCC(c0, c1, c2, c3) ((u32)(c0) | (u32)(c1 << 8) | (u32)(c2 << 16) | (u32)(c3 << 24))
#define LUX_UNUSED(...) ((void)(__VA_ARGS__))

#define IMPL_LX_CONCAT(a, x) a##x
#define LX_CONCAT(a, x) IMPL_LX_CONCAT(a, x)

#endif // #ifndef INCLUDED_HELPER_MACROS_H