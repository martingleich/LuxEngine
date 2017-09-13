#ifndef INCLUDED_QUATERNION_H
#define INCLUDED_QUATERNION_H
#include "math/vector3.h"
#include "core/lxTypes.h"
#include "core/lxFormat.h"

namespace lux
{
namespace math
{

//! Represent a quaternion
template <typename T>
class Quaternion
{
public:
	T x;
	T y;
	T z;
	T w;

	//! default Constructor, Imaginary part is 0 real part is 1
	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{
	}

	//! Constructor
	Quaternion(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W)
	{
	}

	//! Constructor from rotationaxis and rotationangle in rad
	Quaternion(const math::Vector3<T>& axis, Angle<T> Angle)
	{
		*this = FromAngleAxis(Angle, axis);
	}

	//!Copyconstuctor
	Quaternion(const Quaternion& other) :
		x(other.x),
		y(other.y),
		z(other.z),
		w(other.w)
	{
	}

	//! Equality
	bool operator==(const Quaternion<T>& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	//! Inequality
	bool operator!=(const Quaternion<T>& other) const
	{
		return x != other.x || y != other.y || z != other.z || x != other.w;
	}

	//! Assignment
	Quaternion<T>& operator=(const Quaternion<T>& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;

		return *this;
	}

	//! Short addition
	Quaternion<T>& operator+=(const Quaternion<T>& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	//! Addition
	Quaternion<T> operator+(const Quaternion<T>& other) const
	{
		return Quaternion<T>(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	//! Short subtraction
	Quaternion<T>& operator-=(const Quaternion<T>& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
	}

	//! Subtraction
	Quaternion<T> operator-(const Quaternion<T>& other) const
	{
		return Quaternion<T>(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	//! Short multiplication
	Quaternion<T>& operator*=(const Quaternion<T>& other)
	{
		float tmp[4];
		tmp[0] = other.w*w - other.x*x - other.y*y - other.z*z;
		tmp[1] = other.w*x + other.x*w + other.y*z - other.z*y;
		tmp[2] = other.w*y + other.y*w + other.z*x - other.x*z;
		tmp[3] = other.w*z + other.z*w + other.x*y - other.y*x;

		return this->Set(tmp[1], tmp[2], tmp[3], tmp[0]);
	}

	//! Multiplication
	Quaternion<T> operator*(const Quaternion<T>& other) const
	{
		Quaternion<T> tmp = *this;
		tmp *= other;

		return tmp;
	}

	//! Scalar multiplication
	Quaternion<T> operator*(T s) const
	{
		return Quaternion<T>(x*s, y*s, z*s, w*s);
	}

	//! Short scalar multiplication
	Quaternion<T>& operator*=(T s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;

		return *this;
	}

	//! Negate the quaternion
	Quaternion<T> operator-() const
	{
		return Quaternion<T>(-x, -y, -z, -w);
	}

	//! Conjugate this quaternion
	/**
	\return Selfreference
	*/
	Quaternion<T>& Conjungate()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}

	//! Get the conjugate of this quaternion
	Quaternion<T> GetConjungate() const
	{
		return Quaternion<T>(-x, -y, -z, w);
	}

	//! Invert this quaternion
	/**
	\return Selfreference
	*/
	Quaternion<T>& Invert()
	{
		Conjungate();
		*this *= 1 / GetLengthSq();
		return *this;
	}

	//! Get the inverse of this quaternion
	Quaternion<T> GetInverse() const
	{
		Quaternion<T> tmp = *this;
		return tmp.Invert();
	}

	//! Set this quaternion parameter wise
	/**
	\return Selfreference
	*/
	Quaternion<T>& Set(T _x, T _y, T _z, T _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;

		return *this;
	}

	//! Normalize this quaternion
	/**
	\return Selfreference
	*/
	Quaternion<T>& Normalize()
	{
		T l = x*x + y*y + z*z + w*w;
		if(IsZero(l))
			return *this;

		l = 1 / (T)sqrt(l);
		*this *= l;
		return *this;
	}

	//! Get the normal of this quaternion
	/**
	\return The normal of this quaternion
	*/
	Quaternion<T> Normal() const
	{
		Quaternion<T> tmp = *this;
		return tmp.Normalize();
	}

	//! Calculate the dot product with another quaternion
	/**
	\return The dotproduct between this and other
	*/
	T Dot(const Quaternion<T>& other) const
	{
		return x*other.x + y*other.y + z*other.z + w*other.w;
	}

	//! Make this quaternion the identity quaternino
	/**
	\return Selfreference
	*/
	Quaternion<T>& MakeIdent()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 1;
		return *this;
	}

	//! Get the length of this quaternion
	/**
	\return The length of the quaternion
	*/
	T GetLength() const
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}

	//! Get the squared length of this quaternion
	/**
	Much faster than GetLength()
	\return The squared length of the quaternion
	*/
	T GetLengthSq() const
	{
		return x*x + y*y + z*z + w*w;
	}

	//! Build this quaternion form a rotation angle, and a rotation axis
	/**
	\param angle The rotation angle in rad
	\param axis The rotation axis
	\return A reference to this quaternion
	*/
	static Quaternion FromAngleAxis(Angle<T> angle, const Vector3<T>& axis)
	{
		T length = axis.GetLength();
		if(IsZero(length))
			return Quaternion();

		Quaternion<T> out;
		T invLength = 1 / length;
		const Angle<T> halfAngle = angle / 2;
		const T s = (T)Sin(halfAngle);

		return Quaternion(
			axis.x * s * invLength,
			axis.y * s * invLength,
			axis.z * s * invLength,
			(T)Cos(halfAngle));
	}

	//! Read the rotation angle and rotation axis from this quaternion
	/**
	\param Angle The rotation angle in rad
	\param Axis The rotation axis
	*/
	void ToAngleAxis(Angle<T>& angle, Vector3<T>& axis) const
	{
		const T scale = GetLength();
		if(IsZero(scale) || w > 1 || w < -1) {
			angle = Angle<T>::ZERO;
			axis.x = 0;
			axis.y = 1;
			axis.z = 0;
		} else {
			const T Invscale = 1 / scale;
			angle = 2 * ArcCos(w);
			axis.x = Invscale*x;
			axis.y = Invscale*y;
			axis.z = Invscale*z;
		}
	}

	//! Make this quaternion from a Eulerrotation
	/**
	The Eulerrotation is specified in (XYZ) and rad
	\param euler The euler rotation
	\return Selfreference
	*/
	static Quaternion<T> FromEuler(const Vector3<T>& euler)
	{
		T angle;
		angle = euler.x / 2;
		const T sr = sin(angle);
		const T cr = cos(angle);

		angle = euler.y / 2;
		const T sp = sin(angle);
		const T cp = cos(angle);

		angle = euler.z / 2;
		const T sy = sin(angle);
		const T cy = cos(angle);

		const T cpcy = cp * cy;
		const T spcy = sp * cy;
		const T cpsy = cp * sy;
		const T spsy = sp * sy;

		Quaternion<T> out(
			sr*cpcy - cr*spsy,
			cr*spcy + sr*cpsy,
			cr*cpsy - sr*spcy,
			cr*cpcy + sr*spsy);

		// An sich sollte, die L‰nge 1 sein, aber weil Rundungsfehler
		// (sin(a)*cos(b)*cos(c)-cos(a)*sin(b)*sin(c))≤ + (cos(a)*sin(b)*cos(c)+sin(a)*cos(b)*sin(c))≤+(cos(a)*cos(b)*sin(c)-sin(a)*sin(b)*cos(c))≤+(cos(a)*cos(b)*cos(c)+sin(a)*sin(b)*sin(c))≤ = 1
		return out.Normal();
	}

	//! Get the Eulerrotation from this quaternion
	/**
	The rotation is specified in XYZ and rad
	\return The Eulerrotation done by this quaternion
	*/
	Vector3<T> ToEuler() const
	{
		const T test = 2 * (y*w - x*z);
		Vector3<T> euler;

		if(IsEqual(test, (T)1)) {
			euler.z = -2 * atan2(x, w);
			euler.x = 0;
			euler.y = math::Constants<T>::half_pi();
		} else if(IsEqual(test, (T)-1)) {
			euler.z = 2 * atan2(x, w);
			euler.x = 0;
			euler.y = -math::Constants<T>::half_pi();
		} else {
			const T sqW = w*w;
			const T sqX = x*x;
			const T sqY = y*y;
			const T sqZ = z*z;

			euler.z = atan2(2 * (x*y + z*w), sqX - sqY - sqZ + sqW);
			euler.x = atan2(2 * (y*z + x*w), -sqX - sqY + sqZ + sqW);
			euler.y = asin(Clamp(test, (T)-1, (T)1));
		}

		return euler;
	}

	//! transform a vector with this quaternion
	/**
	\param v The vector to transform
	\return The transformed vector
	*/
	Vector3<T> Transform(const Vector3<T>& v) const
	{
		Vector3<T> uv, uuv;
		math::Vector3<T> imag(x, y, z);
		uv = imag.Cross(v);
		uuv = imag.Cross(uv);
		uv *= 2 * w;
		uuv *= 2;

		return v + uv + uuv;
	}

	//! transform a vector with the inverse of this quaternion
	/**
	\param v The vector to transform
	\return The transformed vector
	*/
	Vector3<T> TransformInv(const Vector3<T>& v) const
	{
		Vector3<T> uv, uuv;
		Vector3<T> imag(x, y, z);
		uv = imag.Cross(v);
		uuv = imag.Cross(uv);
		uv *= 2 * w;
		uuv *= 2;

		return v - uv + uuv;
	}

	//! Transforms a vector inplace with this quaternion
	/**
	\param v The vector to transform
	\return The transformed vector
	*/
	Vector3<T>& TransformInPlace(Vector3<T>& v) const
	{
		Vector3<T> uv, uuv;
		Vector3<T> imag(x, y, z);
		uv = imag.Cross(v);
		uuv = imag.Cross(uv);
		uv *= 2 * w;
		uuv *= 2;

		v += uv;
		v += uuv;
		return v;
	}

	//! transform a vector inplace with the inverse of this quaternion
	/**
	\param v The vector to transform
	\return The transformed vector
	*/
	Vector3<T>& TransformInPlaceInv(Vector3<T>& v) const
	{
		Vector3<T> uv, uuv;
		Vector3<T> imag(x, y, z);
		uv = imag.Cross(v);
		uuv = imag.Cross(uv);
		uv *= 2 * w;
		uuv *= 2;

		v -= uv;
		v += uuv;

		return v;
	}

	//! Build a new quaternion moving from one coordiante system to angother
	/**
	to_dir and to_up must be orthogornal.
	\param from_dir The direction in the from system
	\param from_up The up in the from system
	\param to_dir The direction in the to system
	\param to_up The up in the to system
	*/
	static Quaternion FromTo(
		const math::Vector3<T>& from_dir, const math::Vector3<T>& from_up,
		const math::Vector3<T>& to_dir, const math::Vector3<T>& to_up)
	{
		auto lu = from_up.Normal(); // localUp
		auto lf = from_dir.Normal(); // localForward
		auto lr = lu.Cross(lf).Normal(); // localRight

		auto noOrthoWorldUp = to_up.Normal();
		auto wf = to_dir.Normal(); // worldForward
		auto wr = noOrthoWorldUp.Cross(wf).Normal(); // worldRight
		auto wu = wf.Cross(wr).Normal(); // worldUp

		/*
		m1 = (worldRight, perpWorldUp, targetDirection) COL_ORDER // World matrix
		m2 = (localRight, localUp, localForward) ROW_ORDER // Inverse Local matrix

		m = m1 * m2 // Final matrix

		m1 =
		wr.x wu.x wf.x
		wr.y wu.y wf.y
		wr.z wu.z wf.z

		m2 =
		lr.x lr.y lr.z
		lu.x lu.y lu.z
		lf.x lf.y lf.z

		m00 = wr.x*lr.x + wu.x*lu.x + wf.x*lf.x
		m11 = wr.y*lr.y + wu.y*lu.y + wf.y*lf.y
		m22 = wr.z*lr.z + wu.z*lu.z + wf.z*lf.z

		m00+m11+m22 = wr*lr + wu*lu + wf*lf

		m21 = wr.z*lr.y + wu.z*lu.y + wf.z*lf.y
		m12 = wr.y*lr.z + wu.y‹lu.z + wf.y*lf.z

		m02 = wr.x*lr.z + wu.x*lu.z + wf.x*lf.z
		m20 = wr.z*lr.x + wu.z*lu.x + wf.z*lf.x

		m10 = wr.y*lr.x + wu.y*lu.x + wf.y*lf.x
		m01 = wr.x*lr.y + wu.x*lu.y + wf.x*lf.y

		w = sqrt(1 + m00 + m11 + m22) / 2
		x = (m21 - m12)/(4w)
		y = (m02 - m20)/(4w)
		z = (m10 - m01)/(4w)
		*/

		math::Quaternion<T> quat;
		quat.w = sqrt(1.0f + wr.Dot(lr) + wu.Dot(lu) + wf.Dot(lf)) / 2;
		const T w4recip = 1 / (4 * quat.w);

		quat.x = (
			wr.z*lr.y + wu.z*lu.y + wf.z*lf.y -
			wr.y*lr.z - wu.y*lu.z - wf.y*lf.z) * w4recip;

		quat.y = (
			wr.x*lr.z + wu.x*lu.z + wf.x*lf.z -
			wr.z*lr.x - wu.z*lu.x - wf.z*lf.x) * w4recip;

		quat.z = (
			wr.y*lr.x + wu.y*lu.x + wf.y*lf.x -
			wr.x*lr.y - wu.x*lu.y - wf.x*lf.y) * w4recip;

		return quat;
	}

	//! Build this quaternion to rotate from into to
	/**
	Creates a quaternion, which applied to from yield to
	\param from The startvector
	\param to The endvector
	\return Selfreference
	*/
	static Quaternion<T> FromTo(
		const Vector3<T>& from,
		const Vector3<T>& to)
	{
		Vector3<T> v0 = from;
		Vector3<T> v1 = to;
		v0.Normalize();
		v1.Normalize();

		const T d = v0.Dot(v1);
		if(d >= 1) {
			return Quaternion();
		} else if(d <= -1.0f) {
			Vector3<T> axis = math::Vector3<T>::UNIT_X;
			axis = axis.Cross(v0);
			if(IsZero(axis.GetLengthSq())) {
				axis.Set(0, 1, 0);
				axis = axis.Cross(v0);
			}

			return Quaternion(axis.x, axis.y, axis.z, 0).Normal();
		}

		const T s = (T)sqrt((1 + d) * 2);
		const T InvS = 1 / s;
		const Vector3<T> c = v0.Cross(v1) * InvS;
		return Quaternion(c.x, c.y, c.z, s / 2).Normal();
	}
};

///\cond INTERNAL
template <typename T>
Quaternion<T> operator*(T s, const Quaternion<T>& q)
{
	return q*s;
}
///\endcond

//! Typedef for quaternion with float precision
typedef Quaternion<float> QuaternionF;

template <typename T>
bool IsEqual(const Quaternion<T>& a, const Quaternion<T>& b, const T tolerance = math::Constants<T>::rounding_error())
{
	return 
		math::IsEqual(a.x, b.x, tolerance) &&
		math::IsEqual(a.y, b.y, tolerance) &&
		math::IsEqual(a.z, b.z, tolerance) &&
		math::IsEqual(a.w, b.w, tolerance);
}

template <typename T>
bool IsZero(const Quaternion<T>& v, const T tolerance = math::Constants<T>::rounding_error())
{
	return 
		math::IsZero(v.x, tolerance) &&
		math::IsZero(v.y, tolerance) &&
		math::IsZero(v.z, tolerance) &&
		math::IsZero(v.w, tolerance);
}

template <typename T>
void conv_data(format::Context& ctx, const Quaternion<T>& v, format::Placeholder& placeholder)
{
	using namespace format;
	placeholder.type = 'a';
	placeholder.hash.Disable();

	ConvertAddString(ctx, format::StringType::Ascii, "[x=", 3);
	conv_data(ctx, v.x, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " y=", 3);
	conv_data(ctx, v.y, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " z=", 3);
	conv_data(ctx, v.z, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " w=", 3);
	conv_data(ctx, v.w, placeholder);

	ConvertAddString(ctx, format::StringType::Ascii, "]", 1);
}

} // !namespace math
namespace core
{
namespace Types
{
LUX_API Type QuaternionF();
}

template<> inline Type GetTypeInfo<math::Quaternion<float>>() { return Types::QuaternionF(); };
}
} // !namespace lux

#endif // !INCLUDED_QUATERNION_H
