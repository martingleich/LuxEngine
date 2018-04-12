#ifndef INCLUDED_LUX_LXMATH_H
#define INCLUDED_LUX_LXMATH_H
#include "core/LuxBase.h"
#include "core/lxTypes.h"
#include "core/lxFormat.h"

#include <cmath>
#include <cfloat>
#include <climits>

namespace lux
{
namespace math
{

///\cond INTERNAL
namespace impl_math
{
template <typename T>
struct BaseConstants
{
	static T pi()
	{
		return (T)3.1415926535897931;
	}
	static T two_pi()
	{
		return (T)(2 * pi());
	}
	static T reciprocal_pi()
	{
		return (T)(1 / pi());
	}
	static T half_pi()
	{
		return (T)(pi() / 2);
	}
	static T e()
	{
		return (T)(2.7182818284590452);
	}
	static T sqrt2()
	{
		return (T)(1.4142135623730950);
	}
	static T reciprocal_sqrt2()
	{
		return (T)(1 / sqrt2());
	}
	static T deg_to_rad()
	{
		return (T)(pi() / 180);
	}
	static T rad_to_deg()
	{
		return (T)(180 / pi());
	}
	static T rounding_error()
	{
		return (T)0;
	}
};
}

///\endcond

//! List of mathematical constants
/**
All constants are static constants
For example to access pi in floating point precision use \code Constants<float>::pi \endcode

Available constants are:<br>
pi<br>
two_pi<br>
reciprocal_pi<br>
half_pi<br>
e<br>
sqrt2<br>
reciprocal_sqrt2<br>
deg_to_rad<br>
rad_to_deg<br>
rounding_error<br>
*/
template <typename T>
struct Constants : impl_math::BaseConstants<T>
{};



/*
const float Constants<float>::rounding_error = 0.000001f;
const float Constants<float>::min = FLT_MIN;
const float Constants<float>::max = FLT_MAX;
const float Constants<float>::epsilon = FLT_EPSILON;

const double Constants<double>::rounding_error = 0.00000001;
const double Constants<double>::min = DBL_MIN;
const double Constants<double>::max = DBL_MAX;
const double Constants<double>::epsilon = DBL_EPSILON;

const int Constants<int>::rounding_error = 0;
const int Constants<int>::min = INT_MIN;
const int Constants<int>::max = INT_MAX;
const unsigned int Constants<int>::umin = 2147483648;
const int Constants<int>::epsilon = 1;
*/

///\cond INTERNAL

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

template <>
struct Constants<float> : impl_math::BaseConstants<float>
{
	static float rounding_error()
	{
		return 0.000001f;
	}
	static float max()
	{
		return FLT_MAX;
	}
	static float min()
	{
		return FLT_MIN;
	}
	static float epsilon()
	{
		return FLT_EPSILON;
	}
};

template <>
struct Constants<double> : impl_math::BaseConstants<double>
{
	static double rounding_error()
	{
		return 0.00000001f;
	}
	static double max()
	{
		return DBL_MAX;
	}
	static double min()
	{
		return DBL_MIN;
	}
	static double epsilon()
	{
		return DBL_EPSILON;
	}
};

template <>
struct Constants<int> : impl_math::BaseConstants<int>
{
	static int rounding_error()
	{
		return 0;
	}
	static int max()
	{
		return INT_MAX;
	}
	static int min()
	{
		return INT_MIN;
	}
	static unsigned int umin()
	{
		return (unsigned int)(INT_MIN);
	}
	static int epsilon()
	{
		return 1;
	}
};

template <>
struct Constants<u8> : impl_math::BaseConstants<u8>
{
	static const u8 rounding_error()
	{
		return 0;
	}
	static const u8 max()
	{
		return 255;
	}
	static const u8 min()
	{
		return 0;
	}
	static const u8 epsilon()
	{
		return 1;
	}
};

template <>
struct Constants<u16> : impl_math::BaseConstants<u16>
{
	static const u16 rounding_error()
	{
		return 2;
	}
	static const u16 max()
	{
		return 65535;
	}
	static const u16 min()
	{
		return 0;
	}
	static const u16 epsilon()
	{
		return 1;
	}
};

template <>
struct Constants<u32> : impl_math::BaseConstants<u32>
{
	static const u32 rounding_error()
	{
		return 0;
	}
	static const u32 max()
	{
		return 0xFFFFFFFF;
	}
	static const u32 min()
	{
		return 0;
	}
	static const u32 epsilon()
	{
		return 1;
	}
};

///\endcond


//! Calculate value+offset, returns false if the result is outside of the range [0, upper_limit],
//! new_value is only set, if the return value is true.
inline bool AddInsideBounds(u32 value, s32 offset, u32 upper_limit, u32& new_value)
{
	if(offset >= 0) {
		u32 x = (u32)offset;
		if(x <= upper_limit && value <= upper_limit - x)
			new_value = value + x;
		else
			return false;
	} else {
		u32 x = (u32)-offset;
		if(x <= value)
			new_value = value - x;
		else
			return false;
	}

	return true;
}

//! Absolute value
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type Abs(T x)
{
	if(x >= (T)0)
		return x;
	return -x;
}

//! Is the value zero within tolerance
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type IsZero(T x, T roundingError = Constants<T>::rounding_error())
{
	return (Abs(x) <= roundingError);
}

//! Is the value between two values.
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type IsInRange(T x, T lower, T upper)
{
	return (x >= lower && x <= upper);
}

//! Are two values equal within tolerance
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type IsEqual(T a, T b, T roundingError = Constants<T>::rounding_error())
{
	if(a == b)
		return true;
	return (Abs(a - b) <= roundingError);
}

//! Convert from radian to degree
template <typename T>
T RadToDeg(T a)
{
	return a * Constants<T>::rad_to_deg();
}

//! Convert from degree to radian
template <typename T>
T DegToRad(T a)
{
	return a * Constants<T>::deg_to_rad();
}

#ifdef Max 
#undef Max
#endif

//! The bigger value of two
template <typename type>
inline type Max(const type& a, const type& b)
{
	return a > b ? a : b;
}

//! The biggest value of three
template <typename type>
inline type Max(const type& a, const type& b, const type& c)
{
	return a < b ? Max(b, c) : Max(a, c);
}

#ifdef Min
#undef Min
#endif
//! The smaller value of two
template <typename type>
inline type Min(const type& a, const type& b)
{
	return a < b ? a : b;
}

//! The smallest value of three
template <typename type>
inline type Min(const type& a, const type& b, const type& c)
{
	return a < b ? Min(a, c) : Min(b, c);
}

//° Clamp a value inside a range
/**
If Value is smaller than Low, it returns Low
If Value is biffer than Hight, it returns Hight
If Value is between Low and High it returns Value
*/
template <typename type>
inline type Clamp(const type& Value, const type& Low, const type& High)
{
	return Min(Max(Low, Value), High);
}

//! Interpolate linear between A and B
/**
If the value is outside the range [0;1] the result is undefined.
\param A The value for t=0
\param B The value for t=1
\param t The interpolation value
\return The interpolated value
*/
template <typename type>
inline type Lerp(const type& A, const type& B, float t)
{
	// Catch some of the really bad errors. Without getting in way.
	lxAssert(t >= -1 && t <= 2);

	return (type)(A*(1 - t) + B*t);
}

//! Interpolate cubic between two values, bound between other two values
/**
If the value is outside the range [0;1] the result is undefined.
\param n0 The value before n1
\param n1 The value for t=0
\param n2 The value for t=1
\param n3 The value after n2
\param t The interpolation value
\return The interpolated value
*/
template <typename T>
inline T CubicInterpolation(const T& n0, const T& n1, const T& n2, const T& n3, float t)
{
	lxAssert(t >= 0.0f && t <= 1.0f);

	T p = (n3 - n2) - (n0 - n1);
	T q = (n0 - n1) - p;
	T r = n2 - n0;
	T s = n1;

	return p * t * t* t + q * t * t + r*t + s;
}

//! Interpolate hermite between two values.
/**
Requierments for T:
Copy constructable.
Addition, Subtraction, Postmultiplication with float.

If the value is outside the range [0;1] the result is undefined.
\param v1 The value for t=0
\param t1 The tangent for t=0
\param v2 The value for t=1
\param t2 The tangent for t=1
\param t The interpolation value
\return The interpolated value
*/
template <typename T>
T InterpolateHermite(
	const T& v1,
	const T& t1,
	const T& v2,
	const T& t2,
	const float x)
{
	const T A(v1*2.0f - v2*2.0f + t1 + t2);
	const T B(v2*3.0f - v1*3.0f - t1*2.0f - t2);

	return A * (x * x * x) + B * (x * x) + t1 * (x)+v1;
}

//! Maps a value onto a cubic S-curve
/**
If the S-curve is named s. Then s(0) = 0, s(1) = 1,
 s'(0) = 0, s'(1) = 1.
\param x The value to map onto the S-curve
\return The mapped value
*/
inline float SCurve3(float x)
{
	return (x*x*(3 - 2 * x));
}

//! Checks if the given number is prime.
/**
T must be an unsigned integral type.
*/
inline bool IsPrime(int x)
{
	/*
	if(x < 2)
		return false;
	if(x == 2)
		return true;
	if(x % 2 == 0)
		return false;
	for(int i = 3; i*i <= x; i += 2) {
		if(x % i == 0)
			return false;
	}
	*/
	if(x == 2 || x == 3)
		return true;
	if(x % 2 == 0 || x % 3 == 0)
		return false;
	int divisor = 6;
	while(divisor * divisor - 2 * divisor + 1 <= x) {
		if(x % (divisor-1) == 0)
			return false;
		if(x % (divisor+1) == 0)
			return false;
		divisor += 6;
	}
	return true;
}

//! Find the next prime after the given number.
/**
T must be an unsigned integral type.
*/
inline int NextPrime(int x)
{
	switch(x) {
	case 0:
	case 1:
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
	case 5:
		return 5;
	}
	if(x % 2 == 0)
		++x;

	while(!IsPrime(x))
		x += 2;

	return x;
}

//! Check if the given number is a power of two
/**
T must be an unsigned integral type.
Zero is considered a power of two.
*/
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
IsPowerOfTwo(T x)
{
	return ((x - 1)&x) == 0;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
HighestBitPos(T value)
{
	T bit = 0;
	while(value) {
		value >>= 1;
		++bit;
	}
	return bit;
}

} // namespace math
} // namespace lux

#endif
