#ifndef INCLUDED_VECTOR3_H
#define INCLUDED_VECTOR3_H
#include "math/vector2.h"
#include "math/angle.h"

namespace lux
{
namespace math
{

//! A threedimensional vector
/**
in cartesian coordinates
*/
template <typename T>
class vector3
{
public:
	T x; //!< The x component of the vector.
	T y; //!< The y component of the vector.
	T z; //!< The z component of the vector.

public:
	static const vector3 UNIT_X;    //!< Constant for the x-axis
	static const vector3 UNIT_Y;    //!< Constant for the y-axis
	static const vector3 UNIT_Z;    //!< Constant for the z-axis
	static const vector3 NEGATIVE_UNIT_X;    //!< Constant for the negative x-axis
	static const vector3 NEGATIVE_UNIT_Y;    //!< Constant for the negative y-axis
	static const vector3 NEGATIVE_UNIT_Z;    //!< Constant for the negative z-axis
	static const vector3 ZERO;    //!< Constant for the nullvector
	static const vector3 UNIT_scale;    //!< Constant for the unitscale-vector(all components are 1)

public:
	//! Defaultconstructor
	/**
	All components are zero
	*/
	vector3() : x(0), y(0), z(0)
	{
	}
	//! Copyconstructor
	vector3(const vector3& v) : x(v.x), y(v.y), z(v.z)
	{
	}
	//! Constructor from 2d vector and depth
	vector3(const vector2<T>& v, float _z) : x(v.x), y(v.y), z(z)
	{
	}
	//! Constructor from componenents
	vector3(const T X, const T Y, const T Z) : x(X), y(Y), z(Z)
	{
	}
	//! Constructor from single value, all componets get this value
	explicit vector3(const T a) : x(a), y(a), z(a)
	{
	}

	//! Copyconstruct from vector3 with other base T.
	template <typename Type2>
	vector3(const vector3<Type2> v) : x((T)v.x), y((T)v.y), z((T)v.z)
	{
	}

	//! Create a new vector from polar coordinates
	/**
	\param alpha The horicontal angle
	\param beta The vertical angle
	\param length The length of the vector
	*/
	static vector3<T> BuildFromPolar(angle<T> alpha, angle<T> beta, T length)
	{
		return vector3<T>(
			Sin(alpha) * Cos(beta) * length,
			Sin(beta) * length,
			Cos(alpha) * Cos(beta) * length);
	}

	//! Negation
	vector3<T> operator-() const
	{
		return vector3<T>(-x, -y, -z);
	}

	//! Assignment
	vector3<T>& operator=(const vector3<T>& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	//! Shortaddition
	vector3<T>& operator+= (const vector3<T>& v)
	{
		x = x + v.x; y = y + v.y; z = z + v.z; return *this;
	}
	//! Shortsubtraction
	vector3<T>& operator-= (const vector3<T>& v)
	{
		x = x - v.x; y = y - v.y; z = z - v.z; return *this;
	}
	//! Shortmultiplication with scalar.
	vector3<T>& operator*= (const T f)
	{
		x = x*f; y = y*f; z = z*f; return *this;
	}
	//! Shortdivision with scalaer
	vector3<T>& operator/= (const T f)
	{
		x = x / f; y = y / f; z = z / f; return *this;
	}

	//! Short componentwise divison with other vector.
	vector3<T>& operator/= (const vector3<T>& v)
	{
		x = x / v.x; y = y / v.y; z = z / v.z; return *this;
	}
	//! Short componentwise multiplication with other vector.
	vector3<T>& operator*= (const vector3<T>& v)
	{
		x = x*v.x; y = y*v.y; z = z*v.z; return *this;
	}

	//! Addition
	vector3<T> operator+ (const vector3<T>& other) const
	{
		return vector3<T>(x + other.x, y + other.y, z + other.z);
	}

	//! Subtraction
	vector3<T> operator- (const vector3<T>& other) const
	{
		return vector3<T>(x - other.x, y - other.y, z - other.z);
	}
	//! Componentwise multiplication with other vector.
	vector3<T> operator* (const vector3<T>& other) const
	{
		return vector3<T>(x*other.x, y*other.y, z*other.z);
	}
	//! Componentwise divison with other vector.
	vector3<T> operator/ (const vector3<T>& other) const
	{
		return vector3<T>(x / other.x, y / other.y, z / other.z);
	}
	//! Multiplication with scalar.
	vector3<T> operator* (const T f) const
	{
		return vector3<T>(x*f, y*f, z*f);
	}
	//! Division with scalar.
	vector3<T> operator/ (const T f) const
	{
		return vector3<T>(x / f, y / f, z / f);
	}


	//! Equality
	bool operator== (const vector3<T>& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	//! Inequality
	bool operator!= (const vector3<T>& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	//! Smaller operator, for sorting.
	bool operator<(const vector3<T>& other) const
	{
		return (x < other.x && x != other.x) ||
			(x == other.x && y < other.y && y != other.y) ||
			(x == other.x && y == other.y && z < other.z && z != other.z);
	}

	//! Access component by id.
	T& operator[](u32 i)
	{
		assert(i < 3);
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

	T operator[](u32 i) const
	{
		assert(i < 3);
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
	vector3<T>& Set(T X, T Y, T Z)
	{
		x = X;
		y = Y;
		z = Z;

		return *this;
	}

	//! Equality with tolerance
	/**
	\param other The vector to compare
	\param tolerance to comparison tolerance
	\return Are this vector and the other equal
	*/
	bool Equal(const vector3<T>& other, const T tolerance = math::Constants<T>::rounding_error()) const
	{
		return math::IsEqual(x, other.x, tolerance) &&
			math::IsEqual(y, other.y, tolerance) &&
			math::IsEqual(z, other.z, tolerance);
	}

	//! The length of the vector
	/**
	\return The length of the vector
	*/
	T GetLength() const
	{
		return (T)(sqrt((double)(x*x + y*y + z*z)));
	}

	//! The squared length of the vector
	/**
	Much faster than GetLength()
	\return The squared length of the vector
	*/
	T GetLengthSq() const
	{
		return x*x + y*y + z*z;
	}

	//! The distance to another point
	/**
	\param v The point to which the distance is calculated
	\return The distance between this and the other point
	*/
	T GetDistanceTo(const vector3<T>& v) const
	{
		return vector3<T>(x - v.x, y - v.y, z - v.z).GetLength();
	}

	//! The squared distance to another point
	/**
	Much faster than GetDistanceTo
	\param v The point to which the distance is calculated
	\return The squered distance between this and the other point
	*/
	T GetDistanceToSq(const vector3<T>& v) const
	{
		return vector3<T>(x - v.x, y - v.y, z - v.z).GetLengthSq();
	}

	//! Normalized vector
	vector3<T> Normal() const
	{
		vector3<T> out(*this);
		out.Normalize();
		return out;
	}

	//! Normalized vector
	vector3<T> Normal_s() const
	{
		vector3<T> out(*this);
		out.Normalize_s();
		return out;
	}

	//! Normalize this vector
	/**
	\return Selfreference
	*/
	vector3<T>& Normalize()
	{
		SetLength(1);
		return *this;
	}

	//! Normalize this vector
	/**
	Checks for length 0
	\return Selfreference
	*/
	vector3<T>& Normalize_s()
	{
		double length = GetLengthSq();
		if(length > 0) {
			length = 1.0 / sqrt(length);
			x *= T(length);
			y *= T(length);
			z *= T(length);
		}
		return *this;
	}

	//! Stretch this vector to a given length
	/**
	\param newLength The new length of the vector
	\return Selfreference
	*/
	vector3<T>& SetLength(T newLength)
	{
		if(isinf(x) || isinf(y) || isinf(z)) {
			if(isinf(x))
				x = (T)(x < 0 ? -1 : 1);
			else
				x = 0;
			if(isinf(y))
				y = (T)(y < 0 ? -1 : 1);
			else
				y = 0;
			if(isinf(z))
				z = (T)(z < 0 ? -1 : 1);
			else
				z = 0;
		}

		double factor = newLength / GetLength();
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
	vector3<T> Cross(const vector3<T>& v)    const
	{
		return vector3<T>(y*v.z - z*v.y,
			z*v.x - x*v.z,
			x*v.y - y*v.x);
	}

	//! Calculate the Dotproduct between two vectors
	/**
	\param v The other vector
	\return The dotproduct between this and the other vector
	*/
	T Dot(const vector3<T>& v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}

	//! Is this vector between two points
	/**
	This vector must be on the line between v1 and v2
	\param v1 The first point
	\param v2 The second point
	\return True if this vector is between v1 and v2 otherwise false
	*/
	bool IsBetweenPoints(const vector3<T>& v1, const vector3<T>& v2) const
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
	angle<T> AngleTo(const vector3<T>& b) const
	{
		return math::ArcCos<T>(Dot(b)) / (T)sqrt(double(GetLengthSq()*b.GetLengthSq()));
	}

	//! Rotate another vector with this one
	/**
	Take this vector contains a eulerroation(XYZ in rad) now the given vector is rotated by this vector
	\param v The vector to rotate
	\return The rotated vector
	*/
	vector3<T> RotToDir(const vector3<T>& v = vector3<T>::UNIT_Z) const
	{
		const double cx = cos(x);
		const double sx = sin(x);
		const double cy = cos(y);
		const double sy = sin(y);
		const double cz = cos(z);
		const double sz = sin(z);

		const double sxsy = sx*sy;
		const double cxsy = cx*sy;

		const double pseudoMatrix[] = {
			(cy*cz), (cy*sz), (-sy),
			(sxsy*cz - cx*sz), (sxsy*sz + cx*cz), (sx*cy),
			(cxsy*cz + sx*sz), (cxsy*sz - sx*cz), (cx*cy)};

		return vector3<T>(
			(T)(v.x * pseudoMatrix[0] +
				v.y * pseudoMatrix[3] +
				v.z * pseudoMatrix[6]),
				(T)(v.x * pseudoMatrix[1] +
					v.y * pseudoMatrix[4] +
					v.z * pseudoMatrix[7]),
					(T)(v.x * pseudoMatrix[2] +
						v.y * pseudoMatrix[5] +
						v.z * pseudoMatrix[8]));
	}

	//! Convert this vector to polar coordinates
	/**
	The output is horicontal angle, vertical angle, Length
	\return The vector in polarcoordinates
	*/
	vector3<T> ToPolar() const
	{
		vector3<T> vOut;

		double length = x*x + y*y + z*z;
		if(length > 0) {
			length = sqrt(length);
			vOut.z = (T)length;

			if(x != 0) {
				vOut.x = (T)(atan2(z, x));
			}
			// x == 0
			else if(z < 0) {
				vOut.x = LX_HALF_PI;
			}
			//else
			// vOut.x = 0;    // vOut.x ist mittels Standardkonstruktor bereits 0

			vOut.y = (T)(acos(y / length));
		}

		return vOut;
	}


	//! Give the rotation needed to rotate (0,0,1) to this vector
	/**
	\return The needed rotation in Eulerangles(XYZ and rad)
	*/
	vector3<T> GetRotAngles() const
	{
		vector3<T> vOut;

		double length = x*x + y*y + z*z;
		if(length > 0) {
			vOut.y = (T)(atan2(x, z));

			if(vOut.y < 0)
				vOut.y += math::Constants<T>::two_pi();
			if(vOut.y >= math::Constants<T>::two_pi())
				vOut.y -= math::Constants<T>::two_pi();

			const double tmp = (double)(sqrt(x*x + z*z));
			vOut.x = (T)(atan2(tmp, double(y)) - math::Constants<T>::half_pi());

			if(vOut.x < 0)
				vOut.x += math::Constants<T>::two_pi();
			if(vOut.x >= math::Constants<T>::two_pi())
				vOut.x -= math::Constants<T>::two_pi();
		}

		return vOut;
	}

	//! Return the minimum component
	T Min() const
	{
		return math::Min(x, y, z);
	}

	//! Return the maximum component
	T Max() const
	{
		return math::Max(x, y, z);
	}

	//! Return the average component
	T Average() const
	{
		return (x + y + z) / 3;
	}

	//! Returns a vector with component either 1, 0, or -1, which point in the same direction as this vector.
	/**
	The angle between the returned vector and this one, is the smallest of all possibles.
	If this vector is the null vector, the null vector is returned.
	This function works with infinite vectors.
	*/
	vector3 GetUnitCubeVector() const
	{
		T ax = abs(x), ay = abs(y), az = abs(z);
		T max = math::Max(ax, ay, az);
		if(max == 0)
			return vector3::ZERO;

		vector3 out;
		if(max == ax)
			out.x = x < 0 ? (T)-1 : (T)1;
		if(max == ay)
			out.y = y < 0 ? (T)-1 : (T)1;
		if(max == az)
			out.z = z < 0 ? (T)-1 : (T)1;

		return out;
	}
};

///\cond INTERNAL
template <typename T>
inline vector3<T> operator* (const T f, const vector3<T>& v)
{
	return v*f;
}

template <typename T>
inline vector3<T> operator/ (const T f, const vector3<T>& v)
{
	return vector3<T>(f / v.x, f / v.y, f / v.z);
}
///\endcond

//! A 3d vector with float precision
typedef vector3<float> vector3f;
//! A 3d vector with integer precision
typedef vector3<s32>   vector3i;

template <typename T>
const vector3<T>  vector3<T>::UNIT_X = math::vector3<T>(1, 0, 0);
template <typename T>
const vector3<T>  vector3<T>::UNIT_Y = math::vector3<T>(0, 1, 0);
template <typename T>
const vector3<T>  vector3<T>::UNIT_Z = math::vector3<T>(0, 0, 1);
template <typename T>
const vector3<T>  vector3<T>::NEGATIVE_UNIT_X = math::vector3<T>(-1, 0, 0);
template <typename T>
const vector3<T>  vector3<T>::NEGATIVE_UNIT_Y = math::vector3<T>(0, -1, 0);
template <typename T>
const vector3<T>  vector3<T>::NEGATIVE_UNIT_Z = math::vector3<T>(0, 0, -1);
template <typename T>
const vector3<T>  vector3<T>::ZERO = math::vector3<T>(0, 0, 0);
template <typename T>
const vector3<T>  vector3<T>::UNIT_scale = math::vector3<T>(1, 1, 1);

template <typename T>
void conv_data(format::Context& ctx, const vector3<T>& v, format::Placeholder& placeholder)
{
	using namespace format;
	placeholder.type = 'a';
	bool printLength = placeholder.hash.IsEnabled();
	placeholder.hash.Disable();

	ConvertAddString(ctx, StringType::Ascii, "[x=", 3);
	conv_data(ctx, v.x, placeholder);
	ConvertAddString(ctx, StringType::Ascii, " y=", 3);
	conv_data(ctx, v.y, placeholder);
	ConvertAddString(ctx, StringType::Ascii, " z=", 3);
	conv_data(ctx, v.z, placeholder);
	if(printLength) {
		ConvertAddString(ctx, StringType::Ascii, " len=", 5);
		conv_data(ctx, v.GetLength(), placeholder);
	}

	ConvertAddString(ctx, StringType::Ascii, "]", 1);
}

}    // namespace math

const core::Type core::TypeInfo<math::vector3<float>>::typeId = core::Type::Vector3;
const core::Type core::TypeInfo<math::vector3<int>>::typeId = core::Type::Vector3Int;

}    // namespace lux

#endif




