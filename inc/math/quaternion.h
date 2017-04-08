#ifndef INCLUDED_QUATERNION_H
#define INCLUDED_QUATERNION_H
#include "math/vector3.h"

namespace lux
{
namespace math
{

//! Represent a quaternion
template <typename T>
class quaternion
{
public:
	T x;
	T y;
	T z;
	T w;

	//! default Constructor, Imaginary part is 0 real part is 1
	quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{
	}

	//! Constructor
	quaternion(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W)
	{
	}

	//! Constructor from rotationaxis and rotationangle in rad
	quaternion(const math::vector3<T>& axis, angle<T> Angle)
	{
		FromAngleAxis(Angle, axis);
	}

	//!Copyconstuctor
	quaternion(const quaternion& other) :
		x(other.x),
		y(other.y),
		z(other.z),
		w(other.w)
	{
	}

	//! Equality
	bool operator==(const quaternion<T>& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	//! Inequality
	bool operator!=(const quaternion<T>& other) const
	{
		return x != other.x || y != other.y || z != other.z || x != other.w;
	}

	//! Equality with tolerance
	bool Equal(const quaternion<T>& other, const T tolerance = math::Constants<T>::rounding_error()) const
	{
		return math::IsEqual(x, other.x, tolerance) &&
			math::IsEqual(y, other.y, tolerance) &&
			math::IsEqual(z, other.z, tolerance) &&
			math::IsEqual(w, other.w, tolerance);
	}

	//! Assignment
	quaternion<T>& operator=(const quaternion<T>& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;

		return *this;
	}

	//! Short addition
	quaternion<T>& operator+=(const quaternion<T>& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	//! Addition
	quaternion<T> operator+(const quaternion<T>& other) const
	{
		return quaternion<T>(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	//! Short subtraction
	quaternion<T>& operator-=(const quaternion<T>& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
	}

	//! Subtraction
	quaternion<T> operator-(const quaternion<T>& other) const
	{
		return quaternion<T>(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	//! Short multiplication
	quaternion<T>& operator*=(const quaternion<T>& other)
	{
		float tmp[4];
		tmp[0] = other.w*w - other.x*x - other.y*y - other.z*z;
		tmp[1] = other.w*x + other.x*w + other.y*z - other.z*y;
		tmp[2] = other.w*y + other.y*w + other.z*x - other.x*z;
		tmp[3] = other.w*z + other.z*w + other.x*y - other.y*x;

		return this->Set(tmp[1], tmp[2], tmp[3], tmp[0]);
	}

	//! Multiplication
	quaternion<T> operator*(const quaternion<T>& other) const
	{
		quaternion<T> tmp = *this;
		tmp *= other;

		return tmp;
	}

	//! Scalar multiplication
	quaternion<T> operator*(T s) const
	{
		return quaternion<T>(x*s, y*s, z*s, w*s);
	}

	//! Short scalar multiplication
	quaternion<T>& operator*=(T s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;

		return *this;
	}

	//! Negate the quaternion
	quaternion<T> operator-() const
	{
		return quaternion<T>(-x, -y, -z, -w);
	}

	//! Conjugate this quaternion
	/**
	\return Selfreference
	*/
	quaternion<T>& Conjungate()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}

	//! Get the conjugate of this quaternion
	quaternion<T> GetConjungate() const
	{
		return quaternion<T>(-x, -y, -z, w);
	}

	//! Invert this quaternion
	/**
	\return Selfreference
	*/
	quaternion<T>& Invert()
	{
		Conjungate();
		*this *= 1 / GetLengthSq();
		return *this;
	}

	//! Get the inverse of this quaternion
	quaternion<T> GetInverse() const
	{
		quaternion<T> tmp = *this;
		return tmp.Invert();
	}

	//! Set this quaternion parameter wise
	/**
	\return Selfreference
	*/
	quaternion<T>& Set(T _x, T _y, T _z, T _w)
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
	quaternion<T>& Normalize()
	{
		T l = x*x + y*y + z*z + w*w;
		if(l == 1)
			return *this;

		l = 1 / (T)sqrt(l);
		*this *= l;
		return *this;
	}

	//! Get the normal of this quaternion
	/**
	\return The normal of this quaternion
	*/
	quaternion<T> Normal() const
	{
		quaternion<T> tmp = *this;
		return tmp.Normalize();
	}

	//! Calculate the dot product with another quaternion
	/**
	\return The dotproduct between this and other
	*/
	T Dot(const quaternion<T>& other) const
	{
		return x*other.x + y*other.y + z*other.z + w*other.w;
	}

	//! Make this quaternion the identity quaternino
	/**
	\return Selfreference
	*/
	quaternion<T>& MakeIdent()
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
	\param Angle The rotation angle in rad
	\param Axis The rotation axis
	\return A reference to this quaternion
	*/
	static quaternion FromAngleAxis(angle<T> Angle, const vector3<T>& axis)
	{
		T length = axis.GetLength();
		if(IsZero(length))
			return quaternion();

		quaternion<T> out;
		T invLength = 1 / length;
		const angle<T> HalfAngle = Angle / 2;
		const T s = (T)Sin(HalfAngle);

		return quaternion(
			axis.x * s * invLength,
			axis.y * s * invLength,
			axis.z * s * invLength,
			(T)Cos(HalfAngle));
	}

	//! Read the rotation angle and rotation axis from this quaternion
	/**
	\param Angle The rotation angle in rad
	\param Axis The rotation axis
	*/
	void ToAngleAxis(angle<T>& Angle, vector3<T>& axis) const
	{
		const T scale = GetLength();
		if(IsZero(scale) || w > 1 || w < -1) {
			Angle = angle<T>::ZERO;
			axis.x = 0;
			axis.y = 1;
			axis.z = 0;
		} else {
			const T Invscale = 1 / scale;
			Angle = 2 * ArcCos(w);
			axis.x = Invscale*x;
			axis.y = Invscale*y;
			axis.z = Invscale*z;
		}
	}

	//! Make this quaternion from a Eulerrotation
	/**
	The Eulerrotation is specified in (XYZ) and rad
	\param Euler The Euler rotation
	\return Selfreference
	*/
	static quaternion<T> FromEuler(const vector3<T>& Euler)
	{
		T Angle;
		Angle = Euler.x / 2;
		const T sr = sin(Angle);
		const T cr = cos(Angle);

		Angle = Euler.y / 2;
		const T sp = sin(Angle);
		const T cp = cos(Angle);

		Angle = Euler.z / 2;
		const T sy = sin(Angle);
		const T cy = cos(Angle);

		const T cpcy = cp * cy;
		const T spcy = sp * cy;
		const T cpsy = cp * sy;
		const T spsy = sp * sy;

		quaterion out(
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
	vector3<T> ToEuler() const
	{
		const T test = 2 * (y*w - x*z);
		vector3<T> Euler;

		if(Equal(test, (T)1)) {
			Euler.z = -2 * atan2(x, w);
			Euler.x = 0;
			Euler.y = (T)LX_HALF_PI64;
		} else if(Equal(text, (T)-1)) {
			Euler.z = 2 * atan2(x, w);
			Euler.x = 0;
			Euler.y = (T)-LX_HALF_PI64;
		} else {
			const T sqW = w*w;
			const T sqX = x*x;
			const T sqY = y*y;
			const T sqZ = z*z;

			Euler.z = atan2(2 * (x*y + z*w), sqX - sqY - sqZ + sqW);
			Euler.x = atan2(2 * (y*z + x*w), -sqX - sqY + sqZ + sqW);
			Euler.y = asin(Clamp(test, (T)-1, (T)1));
		}

		return Euler;
	}

	//! transform a vector with this quaternion
	/**
	\param v The vector to transform
	\return The transformed vector
	*/
	vector3<T> Transform(const vector3<T>& v) const
	{
		vector3<T> uv, uuv;
		math::vector3<T> imag(x, y, z);
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
	vector3<T> TransformInv(const vector3<T>& v) const
	{
		vector3<T> uv, uuv;
		vector3<T> imag(x, y, z);
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
	vector3<T>& TransformInPlace(vector3<T>& v) const
	{
		vector3<T> uv, uuv;
		vector3<T> imag(x, y, z);
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
	vector3<T>& TransformInPlaceInv(vector3<T>& v) const
	{
		vector3<T> uv, uuv;
		vector3<T> imag(x, y, z);
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
	template <typename T>
	static quaternion<T> FromTo(
		const math::vector3<T>& from_dir, const math::vector3<T>& from_up,
		const math::vector3<T>& to_dir, const math::vector3<T>& to_up)
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

		math::quaternion<T> quat;
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
	static quaternion<T> FromTo(
		const vector3<T>& from,
		const vector3<T>& to)
	{
		vector3<T> v0 = from;
		vector3<T> v1 = to;
		v0.Normalize();
		v1.Normalize();

		const T d = v0.Dot(v1);
		if(d >= 1) {
			return quaternion();
		} else if(d <= -1.0f) {
			vector3<T> axis = math::vector3<T>::UNIT_X;
			axis = axis.Cross(v0);
			if(IsZero(axis.GetLengthSq())) {
				axis.Set(0, 1, 0);
				axis = axis.Cross(v0);
			}

			return quaternion(axis.x, axis.y, axis.z, 0).Normal();
		}

		const T s = (T)sqrt((1 + d) * 2);
		const T InvS = 1 / s;
		const vector3<T> c = v0.Cross(v1) * InvS;
		return quaternion(c.x, c.y, c.z, s / 2).Normal();
	}
};

///\cond INTERNAL
template <typename T>
quaternion<T> operator*(T s, const quaternion<T>& q)
{
	return q*s;
}
///\endcond

//! Typedef for quaternion with float precision
typedef quaternion<float> quaternionf;

} // !namespace math
} // !namespace lux

#endif // !INCLUDED_QUATERNION_H
