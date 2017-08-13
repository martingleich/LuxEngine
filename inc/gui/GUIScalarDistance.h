#ifndef INCLUDED_GUI_SCALAR_DISTANCE_H
#define INCLUDED_GUI_SCALAR_DISTANCE_H
#include "core/LuxBase.h"

namespace lux
{
namespace gui
{

template <typename T>
class ScalarDistance
{
public:
	enum EUnit
	{
		None,
		Pixel,
		Fraction,
		Percent
	};

public:
	T value;
	EUnit unit;

public:
	ScalarDistance(T v) :
		value(v),
		unit(None)
	{
	}
	ScalarDistance(T v, EUnit u) :
		value(v),
		unit(u)
	{
	}

	operator T() const
	{
		return value;
	}

	bool operator==(ScalarDistance other) const { return value == other.value; }
	bool operator<(ScalarDistance other) const { return value < other.value; }
	LX_DEFINE_COMPARE_FUNCTIONS_BY_SMALLER_AND_EQUAL(ScalarDistance);

	T GetRealValue(T base) const
	{
		if(unit == Fraction)
			return value*base;
		if(unit == Percent)
			return (value*base) / 100;
		return value;
	}

	ScalarDistance& operator+=(ScalarDistance other)
	{
		lxAssert(unit == other.unit);
		value += other.value;
		return *this;
	}
	ScalarDistance operator+(ScalarDistance other) const
	{
		lxAssert(unit == other.unit);
		return ScalarDistance(value + other.value, unit);
	}
	ScalarDistance& operator-=(ScalarDistance other)
	{
		lxAssert(unit == other.unit);
		value -= other.value;
		return *this;
	}
	ScalarDistance operator-(ScalarDistance other) const
	{
		lxAssert(unit == other.unit);
		return ScalarDistance(value - other.value, unit);
	}

	ScalarDistance& operator*=(T x)
	{
		value *= x;
		return *this;
	}
	ScalarDistance operator*(T x) const
	{
		return ScalarDistance(value*x, unit);
	}
	ScalarDistance& operator/=(T x)
	{
		value *= x;
		return *this;
	}
	ScalarDistance operator/(T x) const
	{
		return ScalarDistance(value / x, unit);
	}
};

template <typename T>
inline ScalarDistance<T> operator*(T x, ScalarDistance<T>& sd)
{
	return ScalarDistance<T>(sd.value*x, sd.unit);
}

using ScalarDistanceF = ScalarDistance<float>;

template <typename T>
inline ScalarDistanceF Pixel(T value)
{
	return ScalarDistanceF((float)value, ScalarDistanceF::Pixel);
}

inline math::Dimension2<ScalarDistanceF> Pixel(const math::Dimension2F& f)
{
	return math::Dimension2<ScalarDistanceF>(Pixel(f.width), Pixel(f.height));
}
inline ScalarDistanceF Fraction(float value)
{
	return ScalarDistanceF(value, ScalarDistanceF::Fraction);
}
inline ScalarDistanceF Percent(float value)
{
	return ScalarDistanceF(value, ScalarDistanceF::Percent);
}

}
}

#endif // #ifndef INCLUDED_GUI_SCALAR_DISTANCE_H