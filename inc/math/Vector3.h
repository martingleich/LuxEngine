#ifndef INCLUDED_LUX_VECTOR3_H
#define INCLUDED_LUX_VECTOR3_H
#include "math/Vector2.h"
#include "math/Angle.h"

namespace lux
{
namespace math
{

//! A threedimensional vector
/**
in cartesian coordinates
*/
template <typename T>
class Vector3
{
public:
	T x; //!< The x component of the vector.
	T y; //!< The y component of the vector.
	T z; //!< The z component of the vector.

public:
	static const Vector3 UNIT_X;    //!< Constant for the x-axis
	static const Vector3 UNIT_Y;    //!< Constant for the y-axis
	static const Vector3 UNIT_Z;    //!< Constant for the z-axis
	static const Vector3 NEGATIVE_UNIT_X;    //!< Constant for the negative x-axis
	static const Vector3 NEGATIVE_UNIT_Y;    //!< Constant for the negative y-axis
	static const Vector3 NEGATIVE_UNIT_Z;    //!< Constant for the negative z-axis
	static const Vector3 ZERO;    //!< Constant for the nullvector
	static const Vector3 UNIT_scale;    //!< Constant for the unitscale-vector(all components are 1)

public:
	//! Defaultconstructor
	/**
	All components are zero
	*/
	Vector3() : x(0), y(0), z(0)
	{
	}
	//! Copyconstructor
	Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z)
	{
	}
	//! Constructor from 2d vector and depth
	Vector3(const Vector2<T>& v, float _z) : x(v.x), y(v.y), z(z)
	{
	}
	//! Constructor from componenents
	Vector3(const T X, const T Y, const T Z) : x(X), y(Y), z(Z)
	{
	}
	//! Constructor from single value, all componets get this value
	explicit Vector3(const T a) : x(a), y(a), z(a)
	{
	}

	//! Copyconstruct from vector3 with other base T.
	template <typename Type2>
	Vector3(const Vector3<Type2> v) : x((T)v.x), y((T)v.y), z((T)v.z)
	{
	}

	//! Create a new vector from polar coordinates
	/**
	\param alpha The horicontal angle
	\param beta The vertical angle
	\param length The length of the vector
	*/
	static Vector3<T> BuildFromPolar(Angle<T> alpha, Angle<T> beta, T length)
	{
		return Vector3<T>(
			Sin(alpha) * Cos(beta) * length,
			Sin(beta) * length,
			Cos(alpha) * Cos(beta) * length);
	}

	//! Negation
	Vector3<T> operator-() const
	{
		return Vector3<T>(-x, -y, -z);
	}

	//! Assignment
	Vector3<T>& operator=(const Vector3<T>& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	//! Shortaddition
	Vector3<T>& operator+= (const Vector3<T>& v)
	{
		x = x + v.x; y = y + v.y; z = z + v.z; return *this;
	}
	//! Shortsubtraction
	Vector3<T>& operator-= (const Vector3<T>& v)
	{
		x = x - v.x; y = y - v.y; z = z - v.z; return *this;
	}
	//! Shortmultiplication with scalar.
	Vector3<T>& operator*= (const T f)
	{
		x = x * f; y = y * f; z = z * f; return *this;
	}
	//! Shortdivision with scalaer
	Vector3<T>& operator/= (const T f)
	{
		x = x / f; y = y / f; z = z / f; return *this;
	}

	//! Short componentwise divison with other vector.
	Vector3<T>& operator/= (const Vector3<T>& v)
	{
		x = x / v.x; y = y / v.y; z = z / v.z; return *this;
	}
	//! Short componentwise multiplication with other vector.
	Vector3<T>& operator*= (const Vector3<T>& v)
	{
		x = x * v.x; y = y * v.y; z = z * v.z; return *this;
	}

	//! Addition
	Vector3<T> operator+ (const Vector3<T>& other) const
	{
		return Vector3<T>(x + other.x, y + other.y, z + other.z);
	}

	//! Subtraction
	Vector3<T> operator- (const Vector3<T>& other) const
	{
		return Vector3<T>(x - other.x, y - other.y, z - other.z);
	}
	//! Componentwise multiplication with other vector.
	Vector3<T> operator* (const Vector3<T>& other) const
	{
		return Vector3<T>(x*other.x, y*other.y, z*other.z);
	}
	//! Componentwise divison with other vector.
	Vector3<T> operator/ (const Vector3<T>& other) const
	{
		return Vector3<T>(x / other.x, y / other.y, z / other.z);
	}
	//! Multiplication with scalar.
	Vector3<T> operator* (const T f) const
	{
		return Vector3<T>(x*f, y*f, z*f);
	}
	//! Division with scalar.
	Vector3<T> operator/ (const T f) const
	{
		return Vector3<T>(x / f, y / f, z / f);
	}

	//! Equality
	bool operator== (const Vector3<T>& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	//! Inequality
	bool operator!= (const Vector3<T>& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	//! Smaller operator, for sorting.
	bool operator<(const Vector3<T>& other) const
	{
		return (x < other.x && x != other.x) ||
			(x == other.x && y < other.y && y != other.y) ||
			(x == other.x && y == other.y && z < other.z && z != other.z);
	}

	//! Access component by id.
	T& operator[](int i)
	{
		lxAssert(i >= 0 && i < 3);
		switch(i) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return x;
		}
	}

	T operator[](int i) const
	{
		lxAssert(i >= 0 && i < 3);
		switch(i) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return x;
		}
	}


	//! Set the components
	/**
	\param X The new x coordinate
	\param Y The new y coordinate
	\param Z The new z coordinate
	\return Selfreference
	*/
	Vector3<T>& Set(T X, T Y, T Z)
	{
		x = X;
		y = Y;
		z = Z;

		return *this;
	}

	//! The length of the vector
	/**
	\return The length of the vector
	*/
	T GetLength() const
	{
		return (T)(std::sqrt((double)(x*x + y * y + z * z)));
	}

	//! The squared length of the vector
	/**
	Much faster than GetLength()
	\return The squared length of the vector
	*/
	T GetLengthSq() const
	{
		return x * x + y * y + z * z;
	}

	//! The distance to another point
	/**
	\param v The point to which the distance is calculated
	\return The distance between this and the other point
	*/
	T GetDistanceTo(const Vector3<T>& v) const
	{
		return Vector3<T>(x - v.x, y - v.y, z - v.z).GetLength();
	}

	//! The squared distance to another point
	/**
	Much faster than GetDistanceTo
	\param v The point to which the distance is calculated
	\return The squered distance between this and the other point
	*/
	T GetDistanceToSq(const Vector3<T>& v) const
	{
		return Vector3<T>(x - v.x, y - v.y, z - v.z).GetLengthSq();
	}

	//! Normalized vector
	Vector3<T> Normal() const
	{
		Vector3<T> out(*this);
		out.Normalize();
		return out;
	}

	//! Normalize this vector
	/**
	\return Selfreference
	*/
	Vector3<T>& Normalize()
	{
		SetLength(1);
		return *this;
	}

	//! Stretch this vector to a given length
	/**
	\param newLength The new length of the vector
	\return Selfreference
	*/
	Vector3<T>& SetLength(T newLength)
	{
		if(std::isinf(x) || std::isinf(y) || std::isinf(z)) {
			if(std::isinf(x))
				x = (T)(x < 0 ? -1 : 1);
			else
				x = 0;
			if(std::isinf(y))
				y = (T)(y < 0 ? -1 : 1);
			else
				y = 0;
			if(std::isinf(z))
				z = (T)(z < 0 ? -1 : 1);
			else
				z = 0;
		}

		double len = GetLength();
		if(IsZero(len))
			return *this;

		double factor = newLength / len;
		x *= T(factor);
		y *= T(factor);
		z *= T(factor);

		return *this;
	}

	//! Calculate the Crossproduct of two vectors
	/**
	\param v The other vector
	\return The crossproduct between this and the other vector
	*/
	Vector3<T> Cross(const Vector3<T>& v)    const
	{
		return Vector3<T>(
			y*v.z - z * v.y,
			z*v.x - x * v.z,
			x*v.y - y * v.x);
	}

	//! Calculate the Dotproduct between two vectors
	/**
	\param v The other vector
	\return The dotproduct between this and the other vector
	*/
	T Dot(const Vector3<T>& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	//! Is this vector between two points
	/**
	This vector must be on the line between v1 and v2
	\param v1 The first point
	\param v2 The second point
	\return True if this vector is between v1 and v2 otherwise false
	*/
	bool IsBetweenPoints(const Vector3<T>& v1, const Vector3<T>& v2) const
	{
		const T f = (v2 - v1).GetLengthSq();
		return GetDistanceToSq(v1) <= f &&
			GetDistanceToSq(v2) <= f;
	}

	//! The angle to another vector
	/**
	The angle is given in rad
	\param b The vector to which the angle is calculated
	\return The angle between this and b
	*/
	Angle<T> AngleTo(const Vector3<T>& b) const
	{
		return math::ArcCos<T>(Dot(b)) / (T)std::sqrt(double(GetLengthSq()*b.GetLengthSq()));
	}

	//! Convert this vector to polar coordinates
	/**
	The output is horicontal angle, vertical angle, Length
	\return The vector in polarcoordinates
	*/
	void ToPolar(Angle<T>& alpha, Angle<T>& beta, T& length) const
	{
		length = x * x + y * y + z * z;
		if(length > 0) {
			length = (T)std::sqrt(length);

			if(x != 0) {
				alpha = ArcTan2<T>(z, x);
			}
			// x == 0
			else if(z < 0) {
				alpha = Angle<T>::QUATER;
			}
			//else
			// vOut.x = 0;    // vOut.x ist mittels Standardkonstruktor bereits 0

			beta = ArcCos<T>(y / length);
		}
	}

	//! Return the minimum component
	T Min() const { return math::Min(x, y, z); }

	//! Return the maximum component
	T Max() const { return math::Max(x, y, z); }

	//! Return the average component
	T Average() const { return (x + y + z) / 3; }

	Vector3 Absolute() const { return Vector3(abs(x), abs(y), abs(z)); }
};

///\cond INTERNAL
template <typename T>
inline Vector3<T> operator* (const T f, const Vector3<T>& v)
{
	return v * f;
}

template <typename T>
inline Vector3<T> operator/ (const T f, const Vector3<T>& v)
{
	return Vector3<T>(f / v.x, f / v.y, f / v.z);
}
///\endcond

//! A 3d vector with float precision
typedef Vector3<float> Vector3F;
//! A 3d vector with integer precision
typedef Vector3<s32> Vector3I;

template <typename T>
const Vector3<T> Vector3<T>::UNIT_X = math::Vector3<T>(1, 0, 0);
template <typename T>
const Vector3<T> Vector3<T>::UNIT_Y = math::Vector3<T>(0, 1, 0);
template <typename T>
const Vector3<T> Vector3<T>::UNIT_Z = math::Vector3<T>(0, 0, 1);
template <typename T>
const Vector3<T> Vector3<T>::NEGATIVE_UNIT_X = math::Vector3<T>(-1, 0, 0);
template <typename T>
const Vector3<T> Vector3<T>::NEGATIVE_UNIT_Y = math::Vector3<T>(0, -1, 0);
template <typename T>
const Vector3<T> Vector3<T>::NEGATIVE_UNIT_Z = math::Vector3<T>(0, 0, -1);
template <typename T>
const Vector3<T> Vector3<T>::ZERO = math::Vector3<T>(0, 0, 0);
template <typename T>
const Vector3<T> Vector3<T>::UNIT_scale = math::Vector3<T>(1, 1, 1);

template <typename T>
void fmtPrint(format::Context& ctx, const Vector3<T>& v, format::Placeholder& placeholder)
{
	auto pl = format::parser::BasicPlaceholder::Parse(placeholder.format, ctx, placeholder.argId);
	if(pl.hash.IsEnabled())
		format::vformat(ctx, "[x={} y={} z={} len={}]", v.x, v.y, v.z, v.GetLength());
	else
		format::vformat(ctx, "[x={} y={} z={}]", v.x, v.y, v.z);
}

template <typename T>
bool IsEqual(const Vector3<T>& a, const Vector3<T>& b, const T tolerance = math::Constants<T>::rounding_error())
{
	return
		math::IsEqual(a.x, b.x, tolerance) &&
		math::IsEqual(a.y, b.y, tolerance) &&
		math::IsEqual(a.z, b.z, tolerance);
}

template <typename T>
bool IsZero(const Vector3<T>& v, const T tolerance = math::Constants<T>::rounding_error())
{
	return
		math::IsZero(v.x, tolerance) &&
		math::IsZero(v.y, tolerance) &&
		math::IsZero(v.z, tolerance);
}

template <typename T> float* begin(math::Vector3<T>& v) { return &v.x; }
template <typename T> float* end(math::Vector3<T>& v) { return (&v.z) + 1; }
template <typename T> const float* begin(const math::Vector3<T>& v) { return &v.x; }
template <typename T> const float* end(const math::Vector3<T>& v) { return (&v.z) + 1; }

//! Give the rotation needed to rotate (0,0,1) to this vector
/**
\return The needed rotation in Eulerangles(XYZ and rad)
*/
inline EulerAngleF GetVectorRotAngles(const Vector3F& v)
{
	EulerAngleF out;

	float length = v.GetLengthSq();
	if(length > 0) {
		out.y = ArcTan2<float>(v.x, v.z);

		if(out.y < AngleF::ZERO)
			out.y += math::AngleF::FULL;
		if(out.y >= AngleF::FULL)
			out.y -= AngleF::FULL;

		length = std::sqrt(length);
		out.x = ArcTan2<float>(length, v.y) - math::AngleF::QUATER;

		if(out.x < AngleF::ZERO)
			out.x += AngleF::FULL;
		if(out.x >= AngleF::FULL)
			out.x -= AngleF::FULL;
	}

	return out;
}

//! Rotate another vector with this one
/**
Take this vector contains a eulerroation(XYZ in rad) now the given vector is rotated by this vector
\param v The vector to rotate
\return The rotated vector
*/
inline Vector3F RotateVectorEuler(const EulerAngleF& euler, const Vector3F& v = Vector3F::UNIT_Z)
{
	const float cx = Cos(euler.x);
	const float sx = Sin(euler.x);
	const float cy = Cos(euler.y);
	const float sy = Sin(euler.y);
	const float cz = Cos(euler.z);
	const float sz = Sin(euler.z);

	const float sxsy = sx * sy;
	const float cxsy = cx * sy;

	const float pseudoMatrix[] = {
		(cy*cz), (cy*sz), (-sy),
		(sxsy*cz - cx * sz), (sxsy*sz + cx * cz), (sx*cy),
		(cxsy*cz + sx * sz), (cxsy*sz - sx * cz), (cx*cy)};

	return Vector3F(
		v.x * pseudoMatrix[0] + v.y * pseudoMatrix[3] + v.z * pseudoMatrix[6],
		v.x * pseudoMatrix[1] + v.y * pseudoMatrix[4] + v.z * pseudoMatrix[7],
		v.x * pseudoMatrix[2] + v.y * pseudoMatrix[5] + v.z * pseudoMatrix[8]);
}

//! Returns a vector with component either 1, 0, or -1, which point in the same direction as this vector.
/**
The angle between the returned vector and this one, is the smallest of all possibles.
If this vector is the null vector, the null vector is returned.
This function works with infinite vectors.
*/
inline Vector3F GetUnitCubeVector(const Vector3F& v)
{
	float ax = abs(v.x), ay = abs(v.y), az = abs(v.z);
	float max = math::Max(ax, ay, az);
	if(max == 0)
		return Vector3F::ZERO;

	Vector3F out;
	if(max == ax)
		out.x = v.x < 0 ? -1.0f : 1.0f;
	if(max == ay)
		out.y = v.y < 0 ? -1.0f : 1.0f;
	if(max == az)
		out.z = v.z < 0 ? -1.0f : 1.0f;

	return out;
}

//! Returns a vector orthonormal to this one.
/**
i.e. The dot product of the returned vector and this one is 0, and the length of returned vector is 1.
*/
inline Vector3F GetOrthoNormalVector(const math::Vector3F& v)
{
	if(Abs(v.x) + Abs(v.y) > 0)
		return Vector3F(-v.y, v.x, 0) / std::sqrt(v.y*v.y + v.x * v.x);
	else if(Abs(v.y) + Abs(v.z) > 0)
		return Vector3F(0, -v.z, v.y) / std::sqrt(v.y*v.y + v.z * v.z);
	else if(Abs(v.x) + Abs(v.z) > 0)
		return Vector3F(-v.x, 0, v.z) / std::sqrt(v.x*v.x + v.z * v.z);
	else
		return Vector3F(1, 0, 0);
}

} // namespace math

namespace core
{
namespace Types
{
LUX_API Type Vector3F();
LUX_API Type Vector3I();
}

template<> struct TemplType<math::Vector3F> { static Type Get() { return Types::Vector3F(); } };
template<> struct TemplType<math::Vector3I> { static Type Get() { return Types::Vector3I(); } };

} // namespace core

} // namespace lux

#endif
