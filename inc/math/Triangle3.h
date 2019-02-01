#ifndef INCLUDED_LUX_TRIANGLE3D_H
#define INCLUDED_LUX_TRIANGLE3D_H
#include "math/AABBox.h"

namespace lux
{
namespace math
{

//! A triangle in the third dimension
template <typename T>
class Triangle3
{
public:
	Vector3<T> A;    //!< The first point of the triangle
	Vector3<T> B;    //!< The second point of the triangle
	Vector3<T> C;    //!< The third point of the triangle

public:
	//! default-Constructor: All 0 triangle
	Triangle3()
	{
	}
	//! Constructor from 3 points
	Triangle3(
		const Vector3<T>& a,
		const Vector3<T>& b,
		const Vector3<T>& c) :
		A(a),
		B(b),
		C(c)
	{
	}

	bool operator==(const Triangle3<T>& other) const { return other.A == A && other.B == other.B && other.C == C; }
	bool operator!=(const Triangle3<T>& other) const { return other.A != A || other.B != B || other.C != C; }

	//! Set triangle points
	Triangle3<T>& Set(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& c)
	{
		A = a;
		B = b;
		C = c;

		return *this;
	}

	//! The normal of the triangle
	/**
	Computed normal is not normalized
	\return The normal of the triangle
	*/
	Vector3<T> GetNormal() const
	{
		return (B - A).Cross(C - A);
	}

	//! The center of the triangle
	/**
	\return The center of the triangle
	*/
	Vector3<T> GetCenter() const
	{
		return (A + B + C) / 3;
	}

	//! The plane of the triangle
	/**
	Plane is normalized
	\return The plane of the triangle
	*/
	Plane<T> GetPlane() const
	{
		return Plane<T>(A, B, C);
	}

	//! The area of the triangle
	/**
	\return The area of the triangle
	*/
	T GetArea() const
	{
		return (B - A).Cross(C - A).GetLength() / (T)(2);
	}

	T GetCircumference() const
	{
		return (B-A).GetLength() + (A-C).GetLength() + (C-B).GetLength();
	}
};

//! Typedef for triangles with float precision
typedef Triangle3<float> Triangle3F;

} // namespace math
} // namespace lux

#endif