#ifndef INCLUDED_ANGLE_H
#define INCLUDED_ANGLE_H
#include "math/lxMath.h"

namespace lux
{
namespace math
{

template <typename T>
class angle
{
public:
	angle() :
		value(0)
	{
	}

	static angle<T> Radian(T f)
	{
		return angle<T>(f);
	}

	static angle<T> Degree(T f)
	{
		return angle<T>((f / 180.0f) * math::Constants<T>::pi());
	}

	static angle<T> Gon(T f)
	{
		return angle<T>((f / 200.0f) * math::Constants<T>::pi());
	}

	T Radian()
	{
		return value;
	}

	T Degree()
	{
		return ((value / math::Constants<T>::pi()) * 180.0f);
	}

	T Gon()
	{
		return ((value / math::Constants<T>::pi()) * 200.0f);
	}

	angle<T>& Normalize()
	{
		value = fmodf(value, math::Constants<T>::two_pi());
		return *this;
	}

	angle<T> Normal() const
	{
		angle<T> out(*this);
		out.Normalize();
		return out;
	}

	explicit angle(T _value) :
		value(_value)
	{
	}

	static const angle ZERO;
	static const angle QUATER;
	static const angle HALF;
	static const angle FULL;

public:
	T value;
};

template <typename T>
const angle<T>  angle<T>::ZERO = angle<T>(0);

template <typename T>
const angle<T>  angle<T>::QUATER = angle<T>(math::Constants<T>::half_pi());

template <typename T>
const angle<T>  angle<T>::HALF = angle<T>(math::Constants<T>::pi());

template <typename T>
const angle<T>  angle<T>::FULL = angle<T>(math::Constants<T>::two_pi());

template <typename T>
inline bool operator==(angle<T> a, angle<T> b)
{
	return (a.Radian() == b.Radian());
}

template <typename T>
inline bool operator!=(angle<T> a, angle<T> b)
{
	return !(a == b);
}

template <typename T>
inline bool operator<(angle<T> a, angle<T> b)
{
	return (a.Radian() < b.Radian());
}

template <typename T>
inline bool operator>(angle<T> a, angle<T> b)
{
	return (b < a);
}

template <typename T>
inline bool operator<=(angle<T> a, angle<T> b)
{
	return !(a > b);
}

template <typename T>
inline bool operator>=(angle<T> a, angle<T> b)
{
	return !(a < b);
}

template <typename T>
inline angle<T> operator+(angle<T> a, angle<T> b)
{
	return angle<T>::Radian(a.Radian() + b.Radian());
}

template <typename T>
inline angle<T> operator-(angle<T> a, angle<T> b)
{
	return angle<T>::Radian(a.Radian() - b.Radian());
}

template <typename T>
inline T operator/(angle<T> a, angle<T> b)
{
	return (a.Radian() / b.Radian());
}

template <typename T>
inline angle<T> operator%(angle<T> a, angle<T> b)
{
	return angle<T>::Radian(fmodf(a.Radian(), b.Radian()));
}

template <typename T, typename T2>
inline angle<T> operator*(T2 f, angle<T> a)
{
	return angle<T>::Radian(f*a.Radian());
}

template <typename T, typename T2>
inline angle<T> operator*(angle<T> a, T2 f)
{
	return angle<T>::Radian(f*a.Radian());
}

template <typename T, typename T2>
inline angle<T> operator/(angle<T> a, T2 f)
{
	return angle<T>::Radian(a.Radian() / f);
}

template <typename T>
inline angle<T> operator-(angle<T> a)
{
	return angle<T>::Radian(-a.Radian());
}

template <typename T>
inline angle<T>& operator-=(angle<T>& a, angle<T> b)
{
	a = a - b;
	return a;
}

template <typename T>
inline angle<T>& operator+=(angle<T>& a, angle<T> b)
{
	a = a + b;
	return a;
}

template <typename T>
inline angle<T>& operator*=(angle<T>& a, float f)
{
	a = a * f;
	return a;
}

template <typename T>
inline angle<T>& operator/=(angle<T>& a, float f)
{
	a = a / f;
	return a;
}

template <typename T>
inline angle<T>& operator%=(angle<T>& a, angle<T> b)
{
	a = a % b;
	return a;
}
template <typename T>
inline T Cos(angle<T> a)
{
	return cosf(a.Radian());
}

template <typename T>
inline T Sin(angle<T> a)
{
	return sinf(a.Radian());
}

template <typename T>
inline T Tan(angle<T> a)
{
	return tanf(a.Radian());
}

template <typename T>
inline angle<T> ArcCos(T f)
{
	return angle<T>::Radian(acosf(f));
}

template <typename T>
inline angle<T> ArcSin(T f)
{
	return angle<T>::Radian(asinf(f));
}

template <typename T>
inline angle<T> ArcTan(T f)
{
	return angle<T>::Radian(atanf(f));
}

template <typename T>
inline angle<T> ArcTan2(T y, T x)
{
	return angle<T>::Radian(atan2f(y, x));
}

typedef angle<float> anglef;

}
}

#endif // #ifndef INCLUDED_ANGLE_H