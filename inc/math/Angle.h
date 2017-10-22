#ifndef INCLUDED_ANGLE_H
#define INCLUDED_ANGLE_H
#include "math/lxMath.h"

namespace lux
{
namespace math
{

template <typename T>
class Angle
{
public:
	Angle() :
		value(0)
	{
	}

	static Angle<T> Radian(T f)
	{
		return Angle<T>(f);
	}

	static Angle<T> Degree(T f)
	{
		return Angle<T>((f / 180.0f) * math::Constants<T>::pi());
	}

	static Angle<T> Gon(T f)
	{
		return Angle<T>((f / 200.0f) * math::Constants<T>::pi());
	}

	T Radian() const
	{
		return value;
	}

	T Degree() const
	{
		return ((value / math::Constants<T>::pi()) * 180.0f);
	}

	T Gon() const
	{
		return ((value / math::Constants<T>::pi()) * 200.0f);
	}

	Angle<T>& Normalize()
	{
		value = fmodf(value, math::Constants<T>::two_pi());
		return *this;
	}

	Angle<T> Normal() const
	{
		Angle<T> out(*this);
		out.Normalize();
		return out;
	}

	explicit Angle(T _value) :
		value(_value)
	{
	}

	static const Angle ZERO;
	static const Angle QUATER;
	static const Angle HALF;
	static const Angle FULL;

public:
	T value;
};

template <typename T>
const Angle<T> Angle<T>::ZERO = Angle<T>(0);

template <typename T>
const Angle<T> Angle<T>::QUATER = Angle<T>(math::Constants<T>::half_pi());

template <typename T>
const Angle<T> Angle<T>::HALF = Angle<T>(math::Constants<T>::pi());

template <typename T>
const Angle<T> Angle<T>::FULL = Angle<T>(math::Constants<T>::two_pi());

template <typename T>
inline bool operator==(Angle<T> a, Angle<T> b)
{
	return (a.Radian() == b.Radian());
}

template <typename T>
inline bool operator!=(Angle<T> a, Angle<T> b)
{
	return !(a == b);
}

template <typename T>
inline bool operator<(Angle<T> a, Angle<T> b)
{
	return (a.Radian() < b.Radian());
}

template <typename T>
inline bool operator>(Angle<T> a, Angle<T> b)
{
	return (b < a);
}

template <typename T>
inline bool operator<=(Angle<T> a, Angle<T> b)
{
	return !(a > b);
}

template <typename T>
inline bool operator>=(Angle<T> a, Angle<T> b)
{
	return !(a < b);
}

template <typename T>
inline Angle<T> operator+(Angle<T> a, Angle<T> b)
{
	return Angle<T>::Radian(a.Radian() + b.Radian());
}

template <typename T>
inline Angle<T> operator-(Angle<T> a, Angle<T> b)
{
	return Angle<T>::Radian(a.Radian() - b.Radian());
}

template <typename T>
inline T operator/(Angle<T> a, Angle<T> b)
{
	return (a.Radian() / b.Radian());
}

template <typename T>
inline Angle<T> operator%(Angle<T> a, Angle<T> b)
{
	return Angle<T>::Radian(fmodf(a.Radian(), b.Radian()));
}

template <typename T, typename T2>
inline Angle<T> operator*(T2 f, Angle<T> a)
{
	return Angle<T>::Radian(f*a.Radian());
}

template <typename T, typename T2>
inline Angle<T> operator*(Angle<T> a, T2 f)
{
	return Angle<T>::Radian(f*a.Radian());
}

template <typename T, typename T2>
inline Angle<T> operator/(Angle<T> a, T2 f)
{
	return Angle<T>::Radian(a.Radian() / f);
}

template <typename T>
inline Angle<T> operator-(Angle<T> a)
{
	return Angle<T>::Radian(-a.Radian());
}

template <typename T>
inline Angle<T>& operator-=(Angle<T>& a, Angle<T> b)
{
	a = a - b;
	return a;
}

template <typename T>
inline Angle<T>& operator+=(Angle<T>& a, Angle<T> b)
{
	a = a + b;
	return a;
}

template <typename T>
inline Angle<T>& operator*=(Angle<T>& a, float f)
{
	a = a * f;
	return a;
}

template <typename T>
inline Angle<T>& operator/=(Angle<T>& a, float f)
{
	a = a / f;
	return a;
}

template <typename T>
inline Angle<T>& operator%=(Angle<T>& a, Angle<T> b)
{
	a = a % b;
	return a;
}
template <typename T>
inline T Cos(Angle<T> a)
{
	return std::cos(a.Radian());
}

template <typename T>
inline T Sin(Angle<T> a)
{
	return std::sin(a.Radian());
}

template <typename T>
inline T Tan(Angle<T> a)
{
	return std::tan(a.Radian());
}

template <typename T>
inline Angle<T> ArcCos(T f)
{
	return Angle<T>::Radian(std::acos(f));
}

template <typename T>
inline Angle<T> ArcSin(T f)
{
	return Angle<T>::Radian(std::asin(f));
}

template <typename T>
inline Angle<T> ArcTan(T f)
{
	return Angle<T>::Radian(std::atan(f));
}

template <typename T>
inline Angle<T> ArcTan2(T y, T x)
{
	return Angle<T>::Radian(std::atan2(y, x));
}

typedef Angle<float> AngleF;

} // namespace math
} // namespace lux

#endif // #ifndef INCLUDED_ANGLE_H