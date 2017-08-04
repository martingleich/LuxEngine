#ifndef INCLUDED_VECTOR2_H
#define INCLUDED_VECTOR2_H
#include "math/Angle.h"

namespace lux
{
namespace math
{

//! A twodimensional vector
/**
in cartesian coordinates
*/
template <typename T>
class Vector2
{
public:
	T x; //!< The x-coordinate of the vector
	T y; //!< The y-coordinate of the vector

public:
	static const Vector2 UNIT_X;    //!< Constant for the x-axis
	static const Vector2 UNIT_Y;    //!< Constant for the y-axis
	static const Vector2 NEGATIVE_UNIT_X; //!< Constant for the negative x-axis    
	static const Vector2 NEGATIVE_UNIT_Y; //!< Constant for the negative y-axis    
	static const Vector2 ZERO; //!< Constant for the null vector    
	static const Vector2 UNIT_scale; //!< Constant for the unitscale vector(both componets are 1)

public:
	//! Defaultconstructor both components are zero
	Vector2() : x(0), y(0)
	{
	}
	//! Copyconstructor
	Vector2(const Vector2<T>& v) : x(v.x), y(v.y)
	{
	}
	//! Constructor from values
	Vector2(T X, T Y) : x(X), y(Y)
	{
	}
	//! Constructor from single value, both componets get this value
	explicit Vector2(const T a) : x(a), y(a)
	{
	}

	template <typename Type2>
	//! Constructor from other T-vector
	Vector2(const Vector2<Type2> v) : x((T)v.x), y((T)v.y)
	{
	}

	//! Build a new vector from polar coordinates
	/**
	\param Alpha The polar angle in rad
	\param Length The length of the polar vector
	*/
	static Vector2<T> BuildFromPolar(Angle<T> alpha, T length)
	{
		return Vector2<T>(
			(T)(math::Sin(alpha) * length),
			(T)(math::Cos(alpha) * length));
	}

	Vector2<T> operator-() const
	{
		return Vector2<T>(-x, -y);
	}
	Vector2<T>& operator=(const Vector2<T>& v)
	{
		x = v.x; y = v.y; return *this;
	}
	Vector2<T>& operator+= (const Vector2<T>& v)
	{
		x = x + v.x; y = y + v.y; return *this;
	}
	Vector2<T>& operator-= (const Vector2<T>& v)
	{
		x = x - v.x; y = y - v.y; return *this;
	}
	Vector2<T>& operator*= (T f)
	{
		x = x*f; y = y*f; return *this;
	}
	Vector2<T>& operator/= (T f)
	{
		x = x / f; y = y / f; return *this;
	}
	Vector2<T>& operator*= (const Vector2<T>& v)
	{
		x = x*v.x; y = y*v.y; return *this;
	}
	Vector2<T>& operator/= (const Vector2<T>& v)
	{
		x = x / v.x; y = y / v.y; return *this;
	}
	Vector2<T> operator+ (const Vector2<T>& other) const
	{
		return Vector2<T>(x + other.x, y + other.y);
	}
	Vector2<T> operator- (const Vector2<T>& other) const
	{
		return Vector2<T>(x - other.x, y - other.y);
	}
	Vector2<T> operator* (const Vector2<T>& other) const
	{
		return Vector2<T>(x*other.x, y*other.y);
	}
	Vector2<T> operator/ (const Vector2<T>& other) const
	{
		return Vector2<T>(x / other.x, y / other.y);
	}
	Vector2<T> operator* (T f) const
	{
		return Vector2<T>(x*f, y*f);
	}
	Vector2<T> operator/ (T f) const
	{
		return Vector2<T>(x / f, y / f);
	}

	//! Equality
	bool operator==(const Vector2<T>& other) const
	{
		return x == other.x && y == other.y;
	}

	//! Unequality
	bool operator!=(const Vector2<T>& other) const
	{
		return x != other.x || y != other.y;
	}

	//! Smallercomparsion for sorting.
	bool operator<(const Vector2<T>& other) const
	{
		return (x < other.x && x != other.x) ||
			(x == other.x && y < other.y && y != other.y);
	}

	//! Access by index.
	T& operator[](int i)
	{
		lxAssert(i < 2);
		switch(i) {
		case 0: return x;
		case 1: return y;
		default: return x;
		}
	}

	//! Access by index.
	T operator[](int i) const
	{
		lxAssert(i < 2);
		switch(i) {
		case 0: return x;
		case 1: return y;
		default: return x;
		}
	}

	//! Set from components.
	Vector2<T>& Set(T X, T Y)
	{
		x = X;
		y = Y;

		return *this;
	}

	//! The length of the vector
	/**
	\return The length of the vector
	*/
	T GetLength() const
	{
		return (T)(sqrt((double)(x*x + y*y)));
	}

	//! The squared length of the vector
	/**
	Much faster than GetLength()
	\return The squared the length of the vector
	*/
	T GetLengthSq() const
	{
		return x*x + y*y;
	}

	//! The distance to another point
	/**
	\param v The point to which the distance is calculated.
	\return The distance to point v.
	*/
	T GetDistanceTo(const Vector2<T>& v) const
	{
		return Vector2<T>(x - v.x, y - v.y).GetLength();
	}

	//! The squared distance to another point
	/**
	Much faster than GetDistanceTo()
	\param v The point to which the distance is calculated
	\return The squareddistance to point v.
	*/
	T GetDistanceToSq(const Vector2<T>& v) const
	{
		return Vector2<T>(x - v.x, y - v.y).GetLengthSq();
	}

	Vector2<T> Normal()
	{
		Vector2<T> out(*this);
		return out.Normalize();
	}
	//! Normalize this vector
	/**
	\return A reference to this vector
	*/
	Vector2<T>& Normalize()
	{
		SetLength(1);
		return *this;
	}

	//! Stretch the vector to the specified length
	/**
	\param newLength The new length of the vector
	\return A reference to this vector
	*/
	Vector2<T>& SetLength(T newLength)
	{
		if(isinf(x) || isinf(y)) {
			if(isinf(x))
				x = (T)(x < 0 ? -1 : 1);
			else
				x = 0;
			if(isinf(y))
				y = (T)(y < 0 ? -1 : 1);
			else
				y = 0;
		}

		double len = GetLength();
		if(IsZero(len))
			return *this;

		double factor = newLength / len;
		x *= T(factor);
		y *= T(factor);
		return *this;
	}

	//! Is this vector between two points
	/**
	\param v1 The first point
	\param v2 The second point
	\return Is the vector between the two points
	*/
	bool IsBetweenPoints(const Vector2<T>& v1, const Vector2<T>& v2) const
	{
		if(v1.x != v2.x) {
			return ((v1.x <= x && v2.x >= x) ||
				(v1.x >= x && v2.x <= x));
		} else {
			return ((v1.y <= y && v2.y >= y) ||
				(v1.y >= y && v2.y <= y));
		}
	}

	//! A vector orthogonal to this one, creating a left-hand-system
	/**
	\return A vector orthogonal to this one, creating a left-hand-system
	*/
	Vector2<T> Cross() const
	{
		return Vector2<T>(y, -x);
	}

	//! The dot product between this vector an another
	/**
	\param v The vector to compute with
	\return The dot product between this vector an another
	*/
	T Dot(const Vector2<T>& v) const
	{
		return x*v.x + y*v.y;
	}

	//! The angle between this vector an another
	/**
	The angle is in rad
	\param b The vector to which the angle is calculated
	\return The Angle between this vector an another
	*/
	math::Angle<T> Angle(const Vector2<T>& b) const
	{
		return math::ArcCos<T>(Dot(b) / (T)sqrt(GetLengthSq()*b.GetLengthSq()));
	}

	//! Convert this vector to polar(rad) coordinates
	/**
	\return The new polar vector
	*/
	Vector2<T> ToPolar() const
	{
		double length = x*x + y*y;
		if(length > 0) {
			return Vector2<T>(atan2(y, x), sqrt(length));
		}

		return Vector2<T>(0.0, 0.0);
	}

	//! Return the minimum component
	T Min() const
	{
		return math::Min(x, y);
	}

	//! Return the maximum component
	T Max() const
	{
		return math::Max(x, y);
	}

	//! Return the average component
	T Average() const
	{
		return (x + y) / 2;
	}
};

///\cond INTERNAL
template <typename T>
inline Vector2<T> operator* (const T f, const Vector2<T>& v)
{
	return v*f;
}

template <typename T>
inline Vector2<T> operator/ (const T f, const Vector2<T>& v)
{
	return Vector2<T>(f / v.x, f / v.y);
}
///\endcond

//! A 2d vector with float precision
typedef Vector2<float> Vector2F;
//! A 2d vector with integer precision
typedef Vector2<s32> Vector2I;

template <typename T>
const Vector2<T>  Vector2<T>::UNIT_X = math::Vector2<T>(1, 0);
template <typename T>
const Vector2<T>  Vector2<T>::UNIT_Y = math::Vector2<T>(0, 1);
template <typename T>
const Vector2<T>  Vector2<T>::NEGATIVE_UNIT_X = math::Vector2<T>(-1, 0);
template <typename T>
const Vector2<T>  Vector2<T>::NEGATIVE_UNIT_Y = math::Vector2<T>(0, -1);
template <typename T>
const Vector2<T>  Vector2<T>::ZERO = math::Vector2<T>(0, 0);
template <typename T>
const Vector2<T>  Vector2<T>::UNIT_scale = math::Vector2<T>(1, 1);

template <typename T>
bool IsEqual(const Vector2<T>& a, const Vector2<T>& b, T tolerance = math::Constants<T>::rounding_error())
{
	return
		math::IsEqual(a.x, b.x, tolerance) &&
		math::IsEqual(a.y, b.y, tolerance);
}

template <typename T>
bool IsZero(const Vector2<T>& v, T tolerance = math::Constants<T>::rounding_error())
{
	return
		math::IsZero(v.x, tolerance) &&
		math::IsZero(v.y, tolerance);
}

template <typename T>
void conv_data(format::Context& ctx, const Vector2<T>& v, format::Placeholder& placeholder)
{
	using namespace format;

	placeholder.type = 'a';
	bool printLength = placeholder.hash.IsEnabled();
	placeholder.hash.Disable();

	ConvertAddString(ctx, format::StringType::Ascii, "[x=", 3);
	conv_data(ctx, v.x, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " y=", 3);
	conv_data(ctx, v.y, placeholder);
	if(printLength) {
		ConvertAddString(ctx, format::StringType::Ascii, " len=", 5);
		conv_data(ctx, v.GetLength(), placeholder);
	}

	ConvertAddString(ctx, format::StringType::Ascii, "]", 1);
}


} // namespace math

namespace core
{
namespace Types
{
LUX_API Type Vector2f();
LUX_API Type Vector2i();
}

template<> inline Type GetTypeInfo<math::Vector2<float>>() { return Types::Vector2f(); };
template<> inline Type GetTypeInfo<math::Vector2<int>>() { return Types::Vector2i(); };
} // namespace core

} // namespace lux

#endif



